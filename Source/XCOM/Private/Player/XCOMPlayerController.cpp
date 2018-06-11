// Fill out your copyright notice in the Description page of Project Settings.

#include "XCOMPlayerController.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "TileManager.h"
#include "Tile.h"
#include "PawnController.h"
#include "Path.h"
#include "Gun.h"
#include "PlayerPawn.h"
#include "PlayerPawnInAiming.h"
#include "CombatWidget.h"
#include "XCOMGameMode.h"

AXCOMPlayerController::AXCOMPlayerController() 
{
}

void AXCOMPlayerController::BeginPlay()
{
	Super::BeginPlay();
	PrimaryActorTick.bCanEverTick = false;

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATileManager::StaticClass(), FoundActors);
	TileManager = Cast<ATileManager>(FoundActors[0]);

	Initialize();
};

void AXCOMPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
};

void AXCOMPlayerController::SetupInputComponent() {
	Super::SetupInputComponent();

	this->InputComponent->BindAction(L"Click", EInputEvent::IE_Pressed, this, &AXCOMPlayerController::OnClick);
	this->InputComponent->BindAction(L"Cancel", EInputEvent::IE_Pressed, this, &AXCOMPlayerController::CancelWithESC);
}

void AXCOMPlayerController::Initialize() {
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATileManager::StaticClass(), FoundActors);
	TileManager = Cast<ATileManager>(FoundActors[0]);

	FoundActors.Empty();
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerPawnInAiming::StaticClass(), FoundActors);
	PawnInAimingSituation[0] = Cast<APlayerPawnInAiming>(FoundActors[0]);
	PawnInAimingSituation[1] = Cast<APlayerPawnInAiming>(FoundActors[1]);


	FoundActors.Empty();
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerPawn::StaticClass(), FoundActors);
	DefaultPlayerPawn = Cast<APlayerPawn>(FoundActors[0]);

	FoundActors.Empty();
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACustomThirdPerson::StaticClass(), FoundActors);
	for (auto ThirdPersonAsActor : FoundActors) 
	{
		ACustomThirdPerson* SingleThirdPerson = Cast<ACustomThirdPerson>(ThirdPersonAsActor);
		SingleThirdPerson->ChangePlayerPawnDelegate.BindDynamic(this, &AXCOMPlayerController::ChangeToDefaultPawn);
		SingleThirdPerson->ChangeViewTargetDelegate.BindDynamic(this, &AXCOMPlayerController::ChangeViewTargetByCharacter);
		SingleThirdPerson->DeadCamDelegate.BindDynamic(this , &AXCOMPlayerController::ChangeToDeathCam);
		SingleThirdPerson->StartActionDelegate.BindDynamic(this, &AXCOMPlayerController::SetInvisibleCombatWidget);
		SingleThirdPerson->AfterMovingDelegate.BindDynamic(this, &AXCOMPlayerController::AfterCharacterMoving);

		if (SingleThirdPerson->GetTeamFlag()) 
		{
			SingleThirdPerson->AfterActionDelegate.AddUniqueDynamic(this, &AXCOMPlayerController::SwitchNextCharacter);
			PlayerCharacters.Add(SingleThirdPerson);
		}
	}

	if (CombatWidgetBlueprint)
	{
		CombatWidget = CreateWidget<UCombatWidget>(this, CombatWidgetBlueprint);
		if (CombatWidget)
		{
			CombatWidget->AddToViewport();	
			CombatWidget->ChangeViewTargetDelegate.BindDynamic(this, &AXCOMPlayerController::ChangeViewTargetByCombatWidget);
			CombatWidget->StartAttackDelegate.BindDynamic(this, &AXCOMPlayerController::OrderAttack);
			CombatWidget->StartTrajectoryDelegate.BindDynamic(this, &AXCOMPlayerController::OrderStartTrajectory);
			CombatWidget->StartVisilianceDelegate.BindDynamic(this, &AXCOMPlayerController::OrderStartVigilance);
		}
		bShowMouseCursor = true;
	}

	AXCOMGameMode* GameMode = Cast<AXCOMGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
}

void AXCOMPlayerController::OnClick()
{
	FHitResult TraceResult;
	GetHitResultUnderCursor(ECollisionChannel::ECC_MAX, true, TraceResult);
	AActor* actor = TraceResult.GetActor();

	if (!ensure(TraceResult.GetActor())) { return; }
	else
	{
		ACustomThirdPerson* TargetCharacter = Cast<ACustomThirdPerson>(actor);
		ATile* TargetTile = Cast<ATile>(actor);
		if (TargetCharacter)
		{
			if (CheckClickedCharacterTeam(TargetCharacter))
			{
				SwitchCharacter(TargetCharacter);
			}
			else
			{	
				//적클릭시
			}
		}
		else if (TargetTile)
		{
			if (!IsValid(SelectedCharacter))
			{
				UE_LOG(LogTemp, Warning, L"SelectedCharacter invalid!");
				return;
			}

			if (TargetTile->GetTileVisibility() == false)
			{
				UE_LOG(LogTemp, Warning, L"Can not be moved there");
				return;
			}
			
			int32 ActionPointToUse = 0;
			if(TargetTile->bCanMoveWithOneAct) 
			{
				ActionPointToUse = 1;
				SelectedCharacter->CurrentMovableStep /= 2;
			}
			else 
			{
				ActionPointToUse = 2;
			}
			int32 TargetTileIndex = TileManager->ConvertVectorToIndex(TargetTile->GetActorLocation());
//
			TArray<FVector> Tempor;
			for (int32 PathIndex : TileManager->PathArr[TargetTileIndex].OnTheWay) 
			{
				FVector PathLocation = TileManager->ConvertIndexToVector(PathIndex);
				PathLocation += FVector(-50, -50, 0);
				Tempor.Add(PathLocation);
			}
			SelectedCharacter->MoveToTargetTile(&Tempor, ActionPointToUse);

		}
		else
		{
			//TODO 타일과 캐릭터가 아닌 obj 클릭시 처리
		}
	}
}

/**
* 캐릭터가 위치한 타일을 얻어올때 호출합니다.
* @param TargetCharacter
* @return 해당 타일의 포인터
*/
ATile* AXCOMPlayerController::GetOverlappedTile(ACustomThirdPerson* TargetCharacter) 
{
	TArray<AActor*> OverlappedTileArray;
	TargetCharacter->GetOverlappingActors(OverlappedTileArray);

	ATile* OverlappedTile = Cast<ATile>(OverlappedTileArray[0]);

	if (OverlappedTileArray.Num() == 0 || !OverlappedTile)
	{
		return nullptr;
	}
	return OverlappedTile;
}



/**
* 같은 편 안에서 현재 선택되어 있는 캐릭터가 아닌 다른 캐릭터로 전환될때 호출됩니다.
* @param TargetCharacter - 전환할 캐릭터
*/
void AXCOMPlayerController::SwitchCharacter(ACustomThirdPerson* TargetCharacter)
{
	if (TargetCharacter->RemainingActionPoint > 0) {
		DisableInput(this);
		ATile* OverlappedTile = GetOverlappedTile(TargetCharacter);
		if (OverlappedTile == nullptr)		//예외처리 
		{
			EnableInput(this);
			return;
		}

		//클릭시 Actor 이동 필요   ( 카메라 이동은 아님 )
		SelectedCharacter = TargetCharacter;
		SelectedCharacter->ScanEnemy();

		SetTilesToUseSelectedChararacter(OverlappedTile, SelectedCharacter->CurrentMovableStep, SelectedCharacter->GetMovableStepPerActionPoint());

		FPossibleActionWrapper PossibleActionWrapper;
		PossibleActionWrapper.PossibleAction = SelectedCharacter->GetPossibleAction();

		DeleverInfoDelegate.Execute(SelectedCharacter->GetAimingInfo(), PossibleActionWrapper);
	}
	else
	{
		//행동 횟수 0 일때	
	}
}


/**
* 캐릭터가 타일을 이동할 수 있도록 알고리즘 수행과 타일의 Visibility를 세팅합니다.
* @param OverlappedTile - 선택된 캐릭터가 올라가있는 타일
* @param MovingAbility - 이동 가능한 칸 수
* @param MovableStepPerAct - 1 Action point로 이동가능한 칸 수 
*/
void AXCOMPlayerController::SetTilesToUseSelectedChararacter(ATile* OverlappedTile, const int32 MovingAbility,const int32 MovableStepPerAct)
{
	EnableInput(this);
	TileManager->ClearAllTiles(true);

	TArray<ATile*> TilesInRange;
	TileManager->GetAvailableTiles(OverlappedTile, MovingAbility, MovableStepPerAct, TilesInRange);

	for (ATile* Tile : TilesInRange)
	{
		UStaticMeshComponent* TileMesh = Cast<UStaticMeshComponent>(Tile->GetRootComponent());
		if (Tile->bCanMoveWithOneAct) 
		{
			Tile->SelectCloseTileMaterial();
		}
		else 
		{
			Tile->SelectDistantTileMaterial();
		}
		TileMesh->SetVisibility(true);
	}
}

/**
* 캐릭터가 이동을 끝낸후 엄폐 확인, 이동 가능 타일등을 갱신합니다.
* @param MovingCharacter - 이동하는 캐릭터
*/
void AXCOMPlayerController::AfterCharacterMoving(ACustomThirdPerson* MovingCharacter) 
{
	CheckWallAround(MovingCharacter);
	if (MovingCharacter->bCanAction) 
	{
		EnableInput(this);
		TileManager->ClearAllTiles(true);
		if (MovingCharacter->GetTeamFlag()) 
		{
			ATile* OverlappedTile = GetOverlappedTile(MovingCharacter);
			if (OverlappedTile == nullptr)
			{
				return;
			}
			SetTilesToUseSelectedChararacter(OverlappedTile, MovingCharacter->CurrentMovableStep, MovingCharacter->GetMovableStepPerActionPoint());
		}
	}
}

/**
* 캐릭터를 기준으로 4방향으로 주변에 벽이 있는지 확인합니다.
* @param MovingCharacter - 엄폐확인을 할 캐릭터
*/
void AXCOMPlayerController::CheckWallAround(ACustomThirdPerson* TargetCharacter)
{
	if (!TargetCharacter) { return; }

	FVector CharacterPos = TargetCharacter->GetActorLocation();
	int32 CharacterTileIndex = TileManager->ConvertVectorToIndex(CharacterPos);
	UE_LOG(LogTemp, Warning, L"Move To : %d", CharacterTileIndex)

	int32 EastIndex = CharacterTileIndex + 1;
	int32 WestIndex = CharacterTileIndex - 1;
	int32 SouthIndex = CharacterTileIndex - TileManager->GetGridXLength();
	int32 NorthIndex = CharacterTileIndex + TileManager->GetGridXLength();

	CheckWallAroundOneDirection(CharacterTileIndex, EastIndex, TargetCharacter);
	CheckWallAroundOneDirection(CharacterTileIndex, SouthIndex, TargetCharacter);
	CheckWallAroundOneDirection(CharacterTileIndex, NorthIndex, TargetCharacter);
	CheckWallAroundOneDirection(CharacterTileIndex, WestIndex, TargetCharacter);

	if (TargetCharacter->bIsCovering)
	{
		TargetCharacter->RotateTowardWall();
	}

	if (TargetCharacter->bCanAction &&  TargetCharacter->GetTeamFlag())
	{
		TargetCharacter->ScanEnemy();
		FPossibleActionWrapper PossibleActionWrapper;
		PossibleActionWrapper.PossibleAction = TargetCharacter->GetPossibleAction();
		DeleverInfoDelegate.Execute(TargetCharacter->GetAimingInfo(), PossibleActionWrapper);
	}
}


/**
* Cadinal 방향에 대해서 벽이 있는지 확인합니다.
* @param CharacterIndex - 캐릭터가 위치한 타일의 인덱스
* @param CardinalIndex - Cardinal 방향의 타일 인덱스
*/
void AXCOMPlayerController::CheckWallAroundOneDirection(const int32 CharacterIndex, const int CardinalIndex, ACustomThirdPerson* TargetCharacter)
{
	int32 RowNumber = 0;
	EDirection CoverDirection = EDirection::None;
	if (CardinalIndex == (CharacterIndex + TileManager->GetGridXLength())) 
	{
		CoverDirection = EDirection::North; // 북
		RowNumber = 1;
	}
	else if (CardinalIndex == (CharacterIndex - TileManager->GetGridXLength())) 
	{
		CoverDirection = EDirection::South; // 남
		RowNumber = -1;
	}
	else
	{
		if (CardinalIndex == CharacterIndex + 1)
		{
			CoverDirection = EDirection::East; // 동
		}
		else 
		{
			CoverDirection = EDirection::West;	// 서
		}
		RowNumber = 0;
	}

	if (TileManager->CheckWithinBounds(CardinalIndex) && TileManager->IsSameLine(CharacterIndex, RowNumber, CardinalIndex) &&
		TileManager->PathArr[CardinalIndex].bWall) 
	{
		TargetCharacter->CoverDirection = CoverDirection;
		TargetCharacter->CoverDirectionMap.Add(CoverDirection, TileManager->PathArr[CardinalIndex].CoverInfo);
		TargetCharacter->bIsCovering = true;
	}
}

// 테스트를 위한 메소드
// 이후에 삭제 또는 수정 필요
bool AXCOMPlayerController::CheckClickedCharacterTeam(ACustomThirdPerson* ClickedCharacter) 
{
	if (!SelectedCharacter) 
	{ 
		SelectedCharacter = ClickedCharacter;
		return true; 
	}

	if (SelectedCharacter->GetTeamFlag()  == ClickedCharacter->GetTeamFlag())
	{
		return true;
	}
	else if(!(SelectedCharacter->bCanAction))
	{
		return true;
	}
	return false;
}

/**
* 기본 RTS 시점으로 돌아갈때 호출합니다.
*/
void AXCOMPlayerController::ChangeToDefaultPawn() 
{
	if (HealthBarVisiblityDelegate.IsBound()) 
	{
		HealthBarVisiblityDelegate.Execute(true);
	}
	CombatWidget->SetVisibility(ESlateVisibility::Visible);
	SetViewTargetWithBlend(DefaultPlayerPawn, 0.5);
};

/**
* 캐릭터가 Action point를 소모 후 RTS 캠을 이동시킬때 호출됩니다.
*/
FVector AXCOMPlayerController::GetNextAvailableCharLocation() 
{
	TArray<ACustomThirdPerson*> AvailableCharacters;
	for (auto SinglePlayerChar : PlayerCharacters)
	{
		if (SinglePlayerChar->bCanAction) 
		{	
			AvailableCharacters.Add(SinglePlayerChar);
		}
	}

	if (AvailableCharacters.Num() == 0) 
	{
		//todo something
		return FVector(0, 0, 0);
	}

	int32 NextIndex = CharacterSwitchIndex % AvailableCharacters.Num();
	CharacterSwitchIndex++;
	SwitchCharacter(AvailableCharacters[NextIndex]);	

	return AvailableCharacters[NextIndex]->GetActorLocation();;
}


void AXCOMPlayerController::ReleaseCharacter() 
{
	TileManager->ClearAllTiles(true);
}

/**
* Action Cam 시점으로 부드럽게 이동합니다.
* @param StartLocation 
* @param TargetLocation
*/
void AXCOMPlayerController::ChangeViewTargetWithBlend(const FVector StartLocation, const FVector TargetLocation)
{
	if (HealthBarVisiblityDelegate.IsBound())
	{
		HealthBarVisiblityDelegate.Execute(false);
	}

	APlayerPawnInAiming* ActionCam = GetNextActionCam();
	ActionCam->SetCameraPositionInAimingSituation(StartLocation, TargetLocation);
	SetViewTargetWithBlend(ActionCam, 0.5);

	TileManager->ClearAllTiles(true);
	bCameraOrder = !bCameraOrder;
	OrderFinishTrajectory();
}

/**
* Combat Widget에 의해 Action Cam 시점으로 부드럽게 이동합니다.
* @param TargetLocation
*/
void AXCOMPlayerController::ChangeViewTargetByCombatWidget(const FVector TargetLocation)
{
	FVector2D ScreenLocation;
	ProjectWorldLocationToScreen(TargetLocation, ScreenLocation);

	CombatWidget->SetAimWidgetLocation(ScreenLocation);

	ChangeViewTargetWithBlend(SelectedCharacter->GetActorLocation(), TargetLocation);
}

/**
* 캐릭터에 의해 Action Cam 시점으로 부드럽게 이동합니다. ( 사격 후 )
* @param TargetLocation
*/
void AXCOMPlayerController::ChangeViewTargetByCharacter(const FVector CharacterLocation, const FVector TargetLocation)
{
	ChangeViewTargetWithBlend(CharacterLocation, TargetLocation);
}

/**
* 캐릭터가 죽을때의 모습을 비출 Action Cam 으로 시점을 바꿉니다.
* @param MurderedCharLocation
*/
void AXCOMPlayerController::ChangeToDeathCam(const FVector MurderedCharLocation)
{
	APlayerPawnInAiming* ActionCam = GetNextActionCam();
	ActionCam->SetDeathCam(SelectedCharacter->GetActorLocation(), MurderedCharLocation);
	//SetViewTargetWithBlend(ActionCam, 0.5); 블렌드
	SetViewTarget(ActionCam);

	TileManager->ClearAllTiles(true);
	bCameraOrder = !bCameraOrder;
}

/**
* 사용될 Action Cam  Actor를 가져옵니다.
* @return APlayerPawnInAiming
*/
APlayerPawnInAiming* AXCOMPlayerController::GetNextActionCam()
{
	if (bCameraOrder) 
	{
		return PawnInAimingSituation[0];
	}
	else 
	{
		return PawnInAimingSituation[1];
	}
}

/**
* Cancel 시킬때 호출됩니다.
*/
void AXCOMPlayerController::CancelWithESC() 
{
	OrderFinishTrajectory();
	//Todo
	SwitchCharacter(SelectedCharacter);

	ChangeToDefaultPawn();
}

/**
* 캐릭터에게 공격을 명령합니다.
* @param TargetEnemyIndex - 공격할 대상의 인덱스
*/
void AXCOMPlayerController::OrderAttack(const int32 TargetEnemyIndex)
{
	SelectedCharacter->AttackEnemyAccoringToIndex(TargetEnemyIndex);
}

/**
* 캐릭터에게 수류탄 궤도 추적을 명령합니다.
*/
void AXCOMPlayerController::OrderStartTrajectory()
{
	TileManager->ClearAllTiles(true);
	ChangeToDefaultPawn();
	SelectedCharacter->StartTrajectory();
}

/**
* 수류탄 궤도 추적을 멈춥니다.
*/
void AXCOMPlayerController::OrderFinishTrajectory()
{
	SelectedCharacter->FinishTrajectory();
}

/**
* 경계 명령을 내립니다.
*/
void AXCOMPlayerController::OrderStartVigilance()
{
	APawnController* CharacterController = Cast<APawnController>(SelectedCharacter->GetController());
	AXCOMGameMode* GameMode = Cast<AXCOMGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	bool OppositeTeamFlag = !SelectedCharacter->GetTeamFlag();

	SelectedCharacter->BindVigilanceEvent(GameMode->GetTeamMemeber(OppositeTeamFlag));
	SelectedCharacter->bInVisilance = true;
	SelectedCharacter->UseActionPoint(2);
	UE_LOG(LogTemp, Warning, L"경계 시작");
}

/**
* Cambat Widget 을 감춥니다.
*/
void AXCOMPlayerController::SetInvisibleCombatWidget()
{
	CombatWidget->SetVisibility(ESlateVisibility::Collapsed);
}
