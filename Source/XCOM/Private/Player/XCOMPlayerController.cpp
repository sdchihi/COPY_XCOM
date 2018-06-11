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
				//��Ŭ����
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
			//TODO Ÿ�ϰ� ĳ���Ͱ� �ƴ� obj Ŭ���� ó��
		}
	}
}

/**
* ĳ���Ͱ� ��ġ�� Ÿ���� ���ö� ȣ���մϴ�.
* @param TargetCharacter
* @return �ش� Ÿ���� ������
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
* ���� �� �ȿ��� ���� ���õǾ� �ִ� ĳ���Ͱ� �ƴ� �ٸ� ĳ���ͷ� ��ȯ�ɶ� ȣ��˴ϴ�.
* @param TargetCharacter - ��ȯ�� ĳ����
*/
void AXCOMPlayerController::SwitchCharacter(ACustomThirdPerson* TargetCharacter)
{
	if (TargetCharacter->RemainingActionPoint > 0) {
		DisableInput(this);
		ATile* OverlappedTile = GetOverlappedTile(TargetCharacter);
		if (OverlappedTile == nullptr)		//����ó�� 
		{
			EnableInput(this);
			return;
		}

		//Ŭ���� Actor �̵� �ʿ�   ( ī�޶� �̵��� �ƴ� )
		SelectedCharacter = TargetCharacter;
		SelectedCharacter->ScanEnemy();

		SetTilesToUseSelectedChararacter(OverlappedTile, SelectedCharacter->CurrentMovableStep, SelectedCharacter->GetMovableStepPerActionPoint());

		FPossibleActionWrapper PossibleActionWrapper;
		PossibleActionWrapper.PossibleAction = SelectedCharacter->GetPossibleAction();

		DeleverInfoDelegate.Execute(SelectedCharacter->GetAimingInfo(), PossibleActionWrapper);
	}
	else
	{
		//�ൿ Ƚ�� 0 �϶�	
	}
}


/**
* ĳ���Ͱ� Ÿ���� �̵��� �� �ֵ��� �˰��� ����� Ÿ���� Visibility�� �����մϴ�.
* @param OverlappedTile - ���õ� ĳ���Ͱ� �ö��ִ� Ÿ��
* @param MovingAbility - �̵� ������ ĭ ��
* @param MovableStepPerAct - 1 Action point�� �̵������� ĭ �� 
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
* ĳ���Ͱ� �̵��� ������ ���� Ȯ��, �̵� ���� Ÿ�ϵ��� �����մϴ�.
* @param MovingCharacter - �̵��ϴ� ĳ����
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
* ĳ���͸� �������� 4�������� �ֺ��� ���� �ִ��� Ȯ���մϴ�.
* @param MovingCharacter - ����Ȯ���� �� ĳ����
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
* Cadinal ���⿡ ���ؼ� ���� �ִ��� Ȯ���մϴ�.
* @param CharacterIndex - ĳ���Ͱ� ��ġ�� Ÿ���� �ε���
* @param CardinalIndex - Cardinal ������ Ÿ�� �ε���
*/
void AXCOMPlayerController::CheckWallAroundOneDirection(const int32 CharacterIndex, const int CardinalIndex, ACustomThirdPerson* TargetCharacter)
{
	int32 RowNumber = 0;
	EDirection CoverDirection = EDirection::None;
	if (CardinalIndex == (CharacterIndex + TileManager->GetGridXLength())) 
	{
		CoverDirection = EDirection::North; // ��
		RowNumber = 1;
	}
	else if (CardinalIndex == (CharacterIndex - TileManager->GetGridXLength())) 
	{
		CoverDirection = EDirection::South; // ��
		RowNumber = -1;
	}
	else
	{
		if (CardinalIndex == CharacterIndex + 1)
		{
			CoverDirection = EDirection::East; // ��
		}
		else 
		{
			CoverDirection = EDirection::West;	// ��
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

// �׽�Ʈ�� ���� �޼ҵ�
// ���Ŀ� ���� �Ǵ� ���� �ʿ�
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
* �⺻ RTS �������� ���ư��� ȣ���մϴ�.
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
* ĳ���Ͱ� Action point�� �Ҹ� �� RTS ķ�� �̵���ų�� ȣ��˴ϴ�.
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
* Action Cam �������� �ε巴�� �̵��մϴ�.
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
* Combat Widget�� ���� Action Cam �������� �ε巴�� �̵��մϴ�.
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
* ĳ���Ϳ� ���� Action Cam �������� �ε巴�� �̵��մϴ�. ( ��� �� )
* @param TargetLocation
*/
void AXCOMPlayerController::ChangeViewTargetByCharacter(const FVector CharacterLocation, const FVector TargetLocation)
{
	ChangeViewTargetWithBlend(CharacterLocation, TargetLocation);
}

/**
* ĳ���Ͱ� �������� ����� ���� Action Cam ���� ������ �ٲߴϴ�.
* @param MurderedCharLocation
*/
void AXCOMPlayerController::ChangeToDeathCam(const FVector MurderedCharLocation)
{
	APlayerPawnInAiming* ActionCam = GetNextActionCam();
	ActionCam->SetDeathCam(SelectedCharacter->GetActorLocation(), MurderedCharLocation);
	//SetViewTargetWithBlend(ActionCam, 0.5); ����
	SetViewTarget(ActionCam);

	TileManager->ClearAllTiles(true);
	bCameraOrder = !bCameraOrder;
}

/**
* ���� Action Cam  Actor�� �����ɴϴ�.
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
* Cancel ��ų�� ȣ��˴ϴ�.
*/
void AXCOMPlayerController::CancelWithESC() 
{
	OrderFinishTrajectory();
	//Todo
	SwitchCharacter(SelectedCharacter);

	ChangeToDefaultPawn();
}

/**
* ĳ���Ϳ��� ������ ����մϴ�.
* @param TargetEnemyIndex - ������ ����� �ε���
*/
void AXCOMPlayerController::OrderAttack(const int32 TargetEnemyIndex)
{
	SelectedCharacter->AttackEnemyAccoringToIndex(TargetEnemyIndex);
}

/**
* ĳ���Ϳ��� ����ź �˵� ������ ����մϴ�.
*/
void AXCOMPlayerController::OrderStartTrajectory()
{
	TileManager->ClearAllTiles(true);
	ChangeToDefaultPawn();
	SelectedCharacter->StartTrajectory();
}

/**
* ����ź �˵� ������ ����ϴ�.
*/
void AXCOMPlayerController::OrderFinishTrajectory()
{
	SelectedCharacter->FinishTrajectory();
}

/**
* ��� ����� �����ϴ�.
*/
void AXCOMPlayerController::OrderStartVigilance()
{
	APawnController* CharacterController = Cast<APawnController>(SelectedCharacter->GetController());
	AXCOMGameMode* GameMode = Cast<AXCOMGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	bool OppositeTeamFlag = !SelectedCharacter->GetTeamFlag();

	SelectedCharacter->BindVigilanceEvent(GameMode->GetTeamMemeber(OppositeTeamFlag));
	SelectedCharacter->bInVisilance = true;
	SelectedCharacter->UseActionPoint(2);
	UE_LOG(LogTemp, Warning, L"��� ����");
}

/**
* Cambat Widget �� ����ϴ�.
*/
void AXCOMPlayerController::SetInvisibleCombatWidget()
{
	CombatWidget->SetVisibility(ESlateVisibility::Collapsed);
}
