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
#include "Components/WidgetComponent.h"
#include "EnemyUnit.h"
#include "Public/Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "ActiveTileIndicator.h"
#include "DestructibleWall.h"


AXCOMPlayerController::AXCOMPlayerController() 
{

}

void AXCOMPlayerController::BeginPlay()
{
	Super::BeginPlay();
	PrimaryActorTick.bCanEverTick = true;

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATileManager::StaticClass(), FoundActors);
	TileManager = Cast<ATileManager>(FoundActors[0]);

	Initialize();
	AimWidget = FindComponentByClass<UWidgetComponent>();
};

void AXCOMPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsFocusing) 
	{
		if (IsValid(FocusedActor)) 
		{
			DefaultPlayerPawn->MoveToTarget(*FocusedActor);
			if (bStopFocusingAutomatically) 
			{
				FVector RTSCameraLocation = DefaultPlayerPawn->GetActorLocation();
				FVector TargetActorLocation = FocusedActor->GetActorLocation();
				float Distance = FVector::Dist2D(RTSCameraLocation, TargetActorLocation);
				if (Distance < 30.f)
				{
					bStopFocusingAutomatically = false;
					DisableFocusing();
				};
			}
		}
	}
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
		SingleThirdPerson->DeadCamDelegate.BindDynamic(this, &AXCOMPlayerController::ChangeToDeathCam);
		SingleThirdPerson->StartActionDelegate.BindDynamic(this, &AXCOMPlayerController::SetInvisibleCombatWidget);
		SingleThirdPerson->AfterMovingDelegate.BindDynamic(this, &AXCOMPlayerController::AfterCharacterMoving);

		SingleThirdPerson->ReadyToAttackDelegate.BindDynamic(this, &AXCOMPlayerController::ChangeToCloseUpCam);
		SingleThirdPerson->StartShootingDelegate.BindDynamic(this, &AXCOMPlayerController::ChangeViewTargetByCombatWidgetWithoutAim);
		if (SingleThirdPerson->GetTeamFlag())
		{
			SingleThirdPerson->AfterActionDelegate.AddUniqueDynamic(this, &AXCOMPlayerController::FocusNextAvailablePlayerUnit);
			//SingleThirdPerson->AfterMovingDelegate.BindDynamic(this, &AXCOMPlayerController::AfterCharacterMoving);

			PlayerCharacters.Add(SingleThirdPerson);
		}
		else 
		{
			AEnemyUnit* Enemy = Cast<AEnemyUnit>(SingleThirdPerson);
			Enemy->PlayAggroEventDelegate.BindDynamic(this, &AXCOMPlayerController::ChangeToFrontCam);
			Enemy->FinishAggroEventDelegate.AddUniqueDynamic(this, &AXCOMPlayerController::ChangeToDefaultPawn);
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

	if (TileIndicatorBlueprint) 
	{
		TileIndicator = GetWorld()->SpawnActor<AActiveTileIndicator>(
			TileIndicatorBlueprint,
			FVector(0, 0, 0),
			FRotator(0, 0, 0)
			);
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

			if (TargetTile->bCanMove == false)
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
			TArray<FVector> Tempor;
			for (int32 PathIndex : TileManager->PathArr[TargetTileIndex].OnTheWay) 
			{
				FVector PathLocation = TileManager->ConvertIndexToVector(PathIndex);
				PathLocation += FVector(-50, -50, 0);
				Tempor.Add(PathLocation);
			}
			FocusedActor = SelectedCharacter;
			EnableFocusing(FocusedActor, false);
			CombatWidget->ConstructWidgetMinimum();
			SelectedCharacter->MoveToTargetTile(&Tempor, ActionPointToUse);
			
		}
		else 
		{
			UE_LOG(LogTemp, Warning, L" %s Is Clicked", *TraceResult.GetActor()->GetName());
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

void AXCOMPlayerController::SwitchCharacter(ACustomThirdPerson* TargetCharacter)
{
	if (TargetCharacter->RemainingActionPoint > 0) {
		DisableInput(this);
		ATile* OverlappedTile = GetOverlappedTile(TargetCharacter);
		if (OverlappedTile == nullptr) 
		{
			EnableInput(this);
			return;
		}
		SelectCharacter(TargetCharacter);
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

void AXCOMPlayerController::SetTilesToUseSelectedChararacter(ATile* OverlappedTile, const int32 MovingAbility,const int32 MovableStepPerAct)
{
	EnableInput(this);
	TileManager->ClearAllTiles(true);

	TArray<ATile*> TilesInRange;
	TileManager->GetAvailableTiles(OverlappedTile, MovingAbility, MovableStepPerAct, TilesInRange);

	TArray<FTransform> DistantTileTrans;
	TArray<FTransform> CloseTileTrans;

	for (ATile* Tile : TilesInRange)
	{
		UStaticMeshComponent* TileMesh = Cast<UStaticMeshComponent>(Tile->GetRootComponent());
		if (Tile->bCanMoveWithOneAct) 
		{
			CloseTileTrans.Add(Tile->GetTransform());
		}
		else 
		{
			DistantTileTrans.Add(Tile->GetTransform());
		}
	}
	TileIndicator->IndicateActiveTiles(CloseTileTrans, DistantTileTrans);
}

void AXCOMPlayerController::AfterCharacterMoving(ACustomThirdPerson* MovingCharacter) 
{
	if (MovingCharacter->GetTeamFlag()) 
	{
		DisableFocusing();
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
				if (MovingCharacter->bCanAction)
				{
					SetTilesToUseSelectedChararacter(OverlappedTile, MovingCharacter->CurrentMovableStep, MovingCharacter->GetMovableStepPerActionPoint());
				}
			}
		}
	}
	CheckWallAround(MovingCharacter);
}

void AXCOMPlayerController::CheckWallAround(ACustomThirdPerson* TargetCharacter) 
{
	if (!TargetCharacter) { return; }

	CheckWallAroundOneDirection(TargetCharacter, EDirection::North);
	CheckWallAroundOneDirection(TargetCharacter, EDirection::South);
	CheckWallAroundOneDirection(TargetCharacter, EDirection::West);
	CheckWallAroundOneDirection(TargetCharacter, EDirection::East);

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

void AXCOMPlayerController::CheckWallAroundOneDirection(ACustomThirdPerson* TargetCharacter, EDirection Direction)
{
	FVector CharacterPos = TargetCharacter->GetActorLocation();
	FVector TargetLocation;
	int32 TileSize = TileManager->GetTileSize();

	int32 CharacterTileIndex = TileManager->ConvertVectorToIndex(CharacterPos);
	int32 CardinalIndex = 0;

	switch (Direction) 
	{
	case EDirection::North:
		CardinalIndex = CharacterTileIndex + TileManager->GetGridXLength();
		TargetLocation = CharacterPos + FVector(0, TileSize, 0);
		break;
	case EDirection::South:
		CardinalIndex = CharacterTileIndex - TileManager->GetGridXLength();
		TargetLocation = CharacterPos + FVector(0, -TileSize, 0);
		break;
	case EDirection::West:
		CardinalIndex = CharacterTileIndex - 1;
		TargetLocation = CharacterPos + FVector(-TileSize, 0, 0);
		break;
	case EDirection::East:
		CardinalIndex = CharacterTileIndex + 1;
		TargetLocation = CharacterPos + FVector(TileSize, 0, 0);
		break;
	default:
		TargetLocation = CharacterPos;
		break;
	}

	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams CollisionParams;
	CollisionParams.TraceTag = TraceTag;
	CollisionParams.bFindInitialOverlaps = false;

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(
		HitResult,
		CharacterPos,
		TargetLocation,
		ECollisionChannel::ECC_GameTraceChannel12,
		CollisionParams
	);

	if (HitResult.GetActor()) 
	{
		ADestructibleWall* Wall = Cast<ADestructibleWall>(HitResult.GetActor());
		if (Wall) 
		{
			Wall->RegisterUnit(TargetCharacter);
		}
		TargetCharacter->CoverDirection = Direction;
		TargetCharacter->CoverDirectionMap.Add(Direction, TileManager->PathArr[CardinalIndex].CoverInfo);
		TargetCharacter->bIsCovering = true;
	}
	
}


// 테스트를 위한 메소드
// 이후에 삭제 또는 수정 필요
bool AXCOMPlayerController::CheckClickedCharacterTeam(ACustomThirdPerson* ClickedCharacter) 
{
	if (!SelectedCharacter) 
	{ 
		SelectCharacter(ClickedCharacter);
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
	AimWidget->SetVisibility(false);
	if (HealthBarVisiblityDelegate.IsBound()) 
	{
		HealthBarVisiblityDelegate.Execute(true);
	}
	CombatWidget->SetVisibility(ESlateVisibility::Visible);
	SetViewTargetWithBlend(DefaultPlayerPawn, 0.5);
};

void AXCOMPlayerController::FocusNextAvailablePlayerUnit(bool bTeam) 
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
		return;
	}

	int32 NextIndex = CharacterSwitchIndex % AvailableCharacters.Num();
	CharacterSwitchIndex++;
	SwitchCharacter(AvailableCharacters[NextIndex]);
	EnableFocusing(AvailableCharacters[NextIndex], true);
}

void AXCOMPlayerController::ChangeViewTarget(const FVector StartLocation, const FVector TargetLocation, bool bPlayBlend = true)
{
	GetCurrentActionCam()->StopCameraMoving();
	if (HealthBarVisiblityDelegate.IsBound())
	{
		HealthBarVisiblityDelegate.Execute(false);
	}

	APlayerPawnInAiming* ActionCam = GetNextActionCam();
	ActionCam->SetCameraPositionInAimingSituation(StartLocation, TargetLocation);
	if (bPlayBlend) 
	{
		SetViewTargetWithBlend(ActionCam, 0.5);
	}
	else 
	{
		SetViewTarget(ActionCam);
	}

	TileManager->ClearAllTiles(true);
	bCameraOrder = !bCameraOrder;
	OrderFinishTrajectory();
}

void AXCOMPlayerController::ChangeViewTargetByCombatWidget(AActor* TargetActor, bool bPlayBlend , int8 InfoIndex)
{
	ChangeViewTarget(SelectedCharacter->GetActorLocation(), TargetActor->GetActorLocation(), bPlayBlend);

	if (SelectedCharacter->GetUnitState() != EUnitState::Attack)
	{
		ACustomThirdPerson* TargetUnit = Cast<ACustomThirdPerson>(TargetActor);
		AimWidget->DetachFromParent();
		AimWidget->SetVisibility(true);
		if (TargetActor)
		{
			AimWidget->AttachTo(TargetActor->GetRootComponent());
			SetAiminigWidgetFactor(SelectedCharacter, TargetUnit, InfoIndex);
		}
	}
}


void AXCOMPlayerController::ChangeViewTargetByCombatWidgetWithoutAim(AActor* TargetActor, bool bPlayBlend = true)
{
	ChangeViewTarget(SelectedCharacter->GetActorLocation(), TargetActor->GetActorLocation(), bPlayBlend);
}

void AXCOMPlayerController::ChangeViewTargetByCharacter(const FVector CharacterLocation, const FVector TargetLocation)
{
	ChangeViewTarget(CharacterLocation, TargetLocation);
}

void AXCOMPlayerController::ChangeToDeathCam(AActor* MurderedActor)
{
	APlayerPawnInAiming* ActionCam = GetNextActionCam();
	ActionCam->SetDeathCam(SelectedCharacter->GetActorLocation(), MurderedActor);
	SetViewTarget(ActionCam);

	TileManager->ClearAllTiles(true);
	bCameraOrder = !bCameraOrder;
}

void AXCOMPlayerController::ChangeToCloseUpCam(AActor* TargetActor, FVector ForwardDirection) 
{
	APlayerPawnInAiming* ActionCam = GetNextActionCam();
	ActionCam->SetCloseUpCam(TargetActor, ForwardDirection);
	SetViewTarget(ActionCam);

	bCameraOrder = !bCameraOrder;
}

void AXCOMPlayerController::ChangeToFrontCam(AActor* TargetActor)
{
	APlayerPawnInAiming* ActionCam = GetNextActionCam();
	ActionCam->SetFrontCam(TargetActor);
	SetViewTarget(ActionCam);
}

APlayerPawnInAiming* AXCOMPlayerController::GetCurrentActionCam()
{
	if (bCameraOrder)
	{
		return PawnInAimingSituation[1];
	}
	else
	{
		return PawnInAimingSituation[0];
	}
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
	SwitchCharacter(SelectedCharacter);
	ChangeToDefaultPawn();
}

void AXCOMPlayerController::OrderAttack(const int32 TargetEnemyIndex)
{
	AimWidget->SetVisibility(false);
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
	SelectedCharacter->AnnounceVisilance();
	SelectedCharacter->UseActionPoint(2);
}

void AXCOMPlayerController::SetInvisibleCombatWidget()
{
	CombatWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void AXCOMPlayerController::EnableFocusing(AActor* ActorToFocus, bool bStopAutoMatically = false)
{
	FocusedActor = ActorToFocus;
	bStopFocusingAutomatically = bStopAutoMatically;

	bIsFocusing = true;
	DefaultPlayerPawn->SetActorTickEnabled(true);
}

void AXCOMPlayerController::DisableFocusing() 
{
	bIsFocusing = false;
	DefaultPlayerPawn->SetActorTickEnabled(true);

}


void AXCOMPlayerController::SetAiminigWidgetFactor(ACustomThirdPerson* AiminigUnit, ACustomThirdPerson* AimedUnit, int8 InfoIndex)
{
	FAimingInfo TargetAimingInfo = AiminigUnit->GetAimingInfo()[InfoIndex];
	AimedUnit->SetHealthBarVisibility(true);;

	UTextBlock* PercentageBox = Cast<UTextBlock>(AimWidget->GetUserWidgetObject()->GetWidgetFromName(FName("Percentage")));
	FString TempString;
	TempString.AppendInt((int32)(TargetAimingInfo.Probability*100));
	TempString.AppendChar('%');

	PercentageBox->SetText(FText::FromString(TempString));
}


void AXCOMPlayerController::FinishPlayerTurn() 
{
	TileManager->ClearAllTiles(true);
	CombatWidget->HideAllWidget();
	TileIndicator->ClearAllTile();
}


void AXCOMPlayerController::HideTileIdicator() 
{
	TileIndicator->SetTileVisibility(false);
}

void AXCOMPlayerController::ShowTileIdicator() 
{
	if (TileIndicator) 
	{
		TileIndicator->SetTileVisibility(true);
	}
}