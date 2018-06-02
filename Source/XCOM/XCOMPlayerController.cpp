// Fill out your copyright notice in the Description page of Project Settings.

#include "XCOMPlayerController.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "TileManager2.h"
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
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATileManager2::StaticClass(), FoundActors);
	TileManager = Cast<ATileManager2>(FoundActors[0]);

	Initialize();
};

//아직 사용 안함
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
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATileManager2::StaticClass(), FoundActors);
	TileManager = Cast<ATileManager2>(FoundActors[0]);

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

		if (SingleThirdPerson->GetTeamFlag()) 
		{
			SingleThirdPerson->AfterMovingDelegate.BindDynamic(this, &AXCOMPlayerController::AfterCharacterMoving);
			SingleThirdPerson->AfterActionDelegate.AddUniqueDynamic(this, &AXCOMPlayerController::SwitchNextCharacter);
			PlayerCharacters.Add(SingleThirdPerson);
		}
	}
	//SwitchCharacter(PlayerCharacters[0]);

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
			{	//적 클릭시			(테스트용 코드- 이후에 옮길것이라서 함수화 하지 않는다.)
				/*PawnInAimingSituation->SetCameraPositionInAimingSituation(SelectedCharacter->GetActorLocation(), TargetCharacter->GetActorLocation());
				TileManager->ClearAllTiles(true);
				SetViewTargetWithBlend(PawnInAimingSituation, 0.5);*/
				//SelectedCharacter->CheckAttackPotential(TargetCharacter);
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
			
			//행동력 소비
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
* 다른 캐릭터를 클릭해 전환됬을때 호출됩니다.
* @param OverlappedTile - 선택된 캐릭터가 올라가있는 타일
* @param MovingAbility - 이동 가능한 칸 수
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

void AXCOMPlayerController::AfterCharacterMoving(ACustomThirdPerson* MovingCharacter) 
{
	CheckWallAround(MovingCharacter);
	if (MovingCharacter->bCanAction) 
	{
		EnableInput(this);
		TileManager->ClearAllTiles(true);

		ATile* OverlappedTile = GetOverlappedTile(MovingCharacter);
		if (OverlappedTile == nullptr)		
		{
			EnableInput(this);
			return;
		}
		SetTilesToUseSelectedChararacter(OverlappedTile, MovingCharacter->CurrentMovableStep, MovingCharacter->GetMovableStepPerActionPoint());

	}
}



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

	//Todo
	if (TargetCharacter->bCanAction)
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

void AXCOMPlayerController::ChangeToDefaultPawn() 
{
	if (HealthBarVisiblityDelegate.IsBound()) 
	{
		HealthBarVisiblityDelegate.Execute(true);
	}
	CombatWidget->SetVisibility(ESlateVisibility::Visible);
	SetViewTargetWithBlend(DefaultPlayerPawn, 0.5);
};

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


void AXCOMPlayerController::ChangeViewTargetByCombatWidget(const FVector TargetLocation)
{
	ChangeViewTargetWithBlend(SelectedCharacter->GetActorLocation(), TargetLocation);
}

void AXCOMPlayerController::ChangeViewTargetByCharacter(const FVector CharacterLocation, const FVector TargetLocation)
{
	ChangeViewTargetWithBlend(CharacterLocation, TargetLocation);
}


void AXCOMPlayerController::ChangeToDeathCam(const FVector MurderedCharLocation)
{
	APlayerPawnInAiming* ActionCam = GetNextActionCam();
	ActionCam->SetDeathCam(SelectedCharacter->GetActorLocation(), MurderedCharLocation);
	//SetViewTargetWithBlend(ActionCam, 0.5); 블렌드
	SetViewTarget(ActionCam);

	TileManager->ClearAllTiles(true);
	bCameraOrder = !bCameraOrder;
}

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


void AXCOMPlayerController::CancelWithESC() 
{
	OrderFinishTrajectory();
	//Todo
	SwitchCharacter(SelectedCharacter);

	ChangeToDefaultPawn();
}

void AXCOMPlayerController::OrderAttack(const int32 TargetEnemyIndex)
{
	SelectedCharacter->AttackEnemyAccoringToIndex(TargetEnemyIndex);
}

void AXCOMPlayerController::OrderStartTrajectory()
{
	TileManager->ClearAllTiles(true);
	ChangeToDefaultPawn();
	SelectedCharacter->StartTrajectory();
}

void AXCOMPlayerController::OrderFinishTrajectory()
{
	SelectedCharacter->FinishTrajectory();
}

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

void AXCOMPlayerController::SetInvisibleCombatWidget()
{
	CombatWidget->SetVisibility(ESlateVisibility::Collapsed);
}
