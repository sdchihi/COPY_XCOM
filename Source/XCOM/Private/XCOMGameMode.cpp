// Fill out your copyright notice in the Description page of Project Settings.

#include "XCOMGameMode.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "EnemyUnit.h"
#include "CustomThirdPerson.h"
#include "XCOMPlayerController.h"
#include "FogOfWarManager.h"
#include "EnemyController.h"
#include "PlayerDetector.h"
#include "Waypoint.h"
#include "FogOfWarComponent.h"
#include "FAimingQueue.h"
#include "FloatingWidget.h"
#include "Runtime/Engine/Public/TimerManager.h"

AXCOMGameMode::AXCOMGameMode()
{
}

AXCOMGameMode::~AXCOMGameMode()
{
	FAimingQueue::Destroy();
}

void AXCOMGameMode::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACustomThirdPerson::StaticClass(), FoundActors);

	for (auto SingleActor : FoundActors)
	{
		ACustomThirdPerson* SingleCharacter = Cast<ACustomThirdPerson>(SingleActor);
		SingleCharacter->AfterActionDelegate.AddUniqueDynamic(this, &AXCOMGameMode::CheckTurnOver);
		SingleCharacter->AnnounceDamageDelegate.BindDynamic(this, &AXCOMGameMode::ShowCombatPopUp);
		SingleCharacter->UnitDeadDelegate.AddUniqueDynamic(this, &AXCOMGameMode::UnRegisterUnit);

		if (SingleCharacter->GetTeamFlag()) 
		{
			PlayerCharacters.Add(SingleCharacter);
		}
		else 
		{
			AEnemyUnit* SingleEnemy = Cast<AEnemyUnit>(SingleCharacter);
			SingleEnemy->FinishAggroEventDelegate.AddUniqueDynamic(this, &AXCOMGameMode::CheckTurnAfterEvent);
			SingleEnemy->RegisterEventDelegate.BindDynamic(this, &AXCOMGameMode::RegisterEventActor);
			if (EnemyTeamMap.Contains(SingleEnemy->GetGroupNumber())) 
			{
				EnemyTeamMap[SingleEnemy->GetGroupNumber()].Add((SingleEnemy));
			}
			else 
			{
				TArray<AEnemyUnit*> EnemyUnitArray;
				EnemyUnitArray.Add(SingleEnemy);
				EnemyTeamMap.Add(SingleEnemy->GetGroupNumber(), EnemyUnitArray);
			}
			EnemyCharacters.Add(SingleCharacter);
		}
	}

	FTimerHandle UnUsedHandle;
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &AXCOMGameMode::InitializeWaypointMap);
	GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 2, false);


	AXCOMPlayerController* PlayerController = Cast<AXCOMPlayerController>(GetWorld()->GetFirstPlayerController());
	PlayerController->HealthBarVisiblityDelegate.BindDynamic(this, &AXCOMGameMode::SetVisibleAllHealthBar);
	if (bGenerateFogOfWar) 
	{
		SpawnFogOfWar();
	}

	if (PopUpBlueprint)
	{
		PopUp = CreateWidget<UFloatingWidget>(GetWorld(), PopUpBlueprint.Get());
		PopUp->AddToViewport();
		PopUp->SetVisibility(ESlateVisibility::Hidden);
		PopUp->SetDesiredSizeInViewport(FVector2D(180, 50));
	}
}

/**
* ���� ���� �������� Ȯ���մϴ�.
* @param bIsPlayerTeam - Ȯ���� ���� �÷���
*/
void AXCOMGameMode::CheckTurnOver(const bool bIsPlayerTeam)
{
	UE_LOG(LogTemp, Warning, L"^�Ͽ��� Ȯ�� ȣ���");

	if (HasEnemyUnitEvent()) 
	{
		TurnBuffer = bIsPlayerTeam;
		ExecuteEnemyEvent();
		return;
	}

	bool bIsEnd = true;
	if (bIsPlayerTeam) 
	{

		for (auto SingleCharacter : PlayerCharacters)
		{
			if (SingleCharacter->bCanAction) 
			{
				bIsEnd = false;				
				break;
			};
		}
	}
	else 
	{
		for (auto SingleEnemyCharacter : EnemyCharacters)
		{
			if (SingleEnemyCharacter->bCanAction)
			{
				bIsEnd = false;
				StartBotActivity();
				break;
			};
		}
	}

	if (bIsEnd)		// �� �ѱ�
	{
		AXCOMPlayerController* PlayerController = Cast<AXCOMPlayerController>(GetWorld()->GetFirstPlayerController());
		if (!PlayerController) { return; }

		if (bIsPlayerTeam) 
		{
			PlayerTurnOver();
		}
		else 
		{
			FTimerHandle UnUsedHandle;
			FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &AXCOMGameMode::EnemyTurnOver);
			GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 2, false);
		}
	}
}

/**
* Player���� ���� �����մϴ�.
*/
void AXCOMGameMode::PlayerTurnOver() 
{
	AXCOMPlayerController* PlayerController = Cast<AXCOMPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PlayerController) { return; }
	RestoreTeamActionPoint(EnemyCharacters);

	DisableInput(PlayerController);
	PlayerController->FinishPlayerTurn();
	UE_LOG(LogTemp, Warning, L"�÷��̾��� �� ���� - > AI�� �� ����");
	SetEnemysPatrolLocation();
	EnemyTurnOrder = 0;
	StartBotActivity();
}

/**
* AI���� ���� �����մϴ�.
*/
void AXCOMGameMode::EnemyTurnOver()
{
	AXCOMPlayerController* PlayerController = Cast<AXCOMPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PlayerController) { return; }

	PlayerController->DisableFocusing();
	EnableInput(PlayerController);
	UE_LOG(LogTemp, Warning, L"AI�� �� ����");
	RestoreTeamActionPoint(PlayerCharacters);
	PlayerController->FocusNextAvailablePlayerUnit(true);
}

/**
* ĳ���͵��� Action point�� ȸ����ŵ�ϴ�.
* @param Characters - ȸ����ų ĳ���͵� �迭
*/
void AXCOMGameMode::RestoreTeamActionPoint(TArray<ACustomThirdPerson*>& Characters) 
{
	for (ACustomThirdPerson* SingleCharacter : Characters)
	{
		SingleCharacter->RestoreActionPoint();
	}
}

/**
* ĳ���͵��� Health bar�� ���ü��� �����մϴ�.
* @param bVisible 
*/
void AXCOMGameMode::SetVisibleAllHealthBar(const bool bVisible)
{
	for (ACustomThirdPerson* SinglePlayerChar : PlayerCharacters)
	{
		SinglePlayerChar->SetHealthBarVisibility(bVisible);
	}
	for (ACustomThirdPerson* SingleEnemyChar : EnemyCharacters)
	{
		SingleEnemyChar->SetHealthBarVisibility(bVisible);
	}
}

/**
* FOW(������ �Ȱ�)�� �����մϴ�.
*/
void AXCOMGameMode::SpawnFogOfWar() 
{
	if (!FogOfWarBP) { return; }

	FogOfWar = GetWorld()->SpawnActor<AFogOfWarManager>(
		FogOfWarBP,
		FVector(0, 0, 0),
		FRotator(0, 0, 0)
		);

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACustomThirdPerson::StaticClass(), FoundActors);

	for (AActor* SingleActor : FoundActors) 
	{
		UFogOfWarComponent* FowComp = SingleActor->FindComponentByClass<UFogOfWarComponent>();
		if (FowComp) 
		{
			FogOfWar->RegisterFowActor(SingleActor);
		}
	}
}

/**
* Player Unit Ȥ�� Ai Unit �迭�� ���ɴϴ�.
* @param bTeam - �÷��̾� / AI
* @return ���� ���� ���� �迭
*/
TArray<ACustomThirdPerson*> AXCOMGameMode::GetTeamMemeber(const bool bTeam)
{
	if (bTeam) 
	{
		return PlayerCharacters;
	}
	else 
	{
		return EnemyCharacters;
	}
};

/**
* �� AI Unit�� Ȱ���� �����մϴ�.
*/
void AXCOMGameMode::StartBotActivity() 
{
	if (EnemyTurnOrder >= EnemyCharacters.Num()) { return; };
	AXCOMPlayerController* PlayerController = Cast<AXCOMPlayerController>(GetWorld()->GetFirstPlayerController());

	ACustomThirdPerson* EnemyChar = EnemyCharacters[EnemyTurnOrder];
	AEnemyController* EnemyController = EnemyChar ? Cast<AEnemyController>(EnemyChar->GetController()) : nullptr;
	if (!EnemyController) 
	{
		EnemyTurnOrder++;
		StartBotActivity();
	}
	else 
	{
		EnemyTurnOrder++;
		PlayerController->SelectCharacter(EnemyChar);
		EnemyController->StartBehaviorTreeFromDefault();
	}
}

/**
* AI Unit�� ���� ��ġ�� �����մϴ�.
*/
void AXCOMGameMode::SetEnemysPatrolLocation()
{
	FVector PlayerUnitsMiddlePoint;
	for (ACustomThirdPerson* SinglePlayerUnit : PlayerCharacters)
	{
		PlayerUnitsMiddlePoint += SinglePlayerUnit->GetActorLocation();
	}
	PlayerUnitsMiddlePoint = PlayerUnitsMiddlePoint / PlayerCharacters.Num();

	for (auto It = EnemyTeamMap.CreateConstIterator(); It; ++It)
	{
		bool bSkipLoop = false;
		
		for (AEnemyUnit* SingleEnemyUnit : It.Value())
		{
			if (IsValid(SingleEnemyUnit)) 
			{
				if (SingleEnemyUnit->IsAggro()) 
				{
					bSkipLoop = true;
					break;
				}
				else { break; }
			}
		}
		if (bSkipLoop) { continue; }

		FVector TargetLocation;
		bool bRenewAfterUsingWaypoint = false;
		if (bEnemyNoticeBattle) 
		{
			TargetLocation = PlayerUnitsMiddlePoint;
		}
		else 
		{
			TargetLocation = WaypointMap[It.Key()]->GetActorLocation();
			bRenewAfterUsingWaypoint = true;
		}

		if (bRenewAfterUsingWaypoint) 
		{
			RenewWaypoint(It.Key());
		}

		for (AEnemyUnit* SingleEnemyUnit : It.Value()) 
		{
			AEnemyController* EnemyUnitController = Cast<AEnemyController>(SingleEnemyUnit->GetController());
			EnemyUnitController->SetPatrolTargetLocation(TargetLocation);
		}
	}
}

/*
* Enemy Group ���� ������ �ϰԵ� Waypoint�� Map���� �����մϴ�
*/
void AXCOMGameMode::InitializeWaypointMap()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWaypoint::StaticClass(), FoundActors);
	for (AActor* SingleWaypoint : FoundActors) 
	{
		WaypointArray.Add(Cast<AWaypoint>(SingleWaypoint));
	}

	for (auto It = EnemyTeamMap.CreateConstIterator(); It; ++It) 
	{
		FVector GroupMiddlePoint;
		for (AEnemyUnit* SingleEnemyUnit : It.Value())
		{
			FVector EnemyLocation = SingleEnemyUnit->GetActorLocation();
			GroupMiddlePoint += EnemyLocation;
		}
		GroupMiddlePoint = GroupMiddlePoint / It.Value().Num();

		float MinDistance = FLT_MAX;
		AWaypoint* NearestWaypoint = nullptr;
		for (AWaypoint* SingleWaypoint : WaypointArray) 
		{
			float Distance = FVector::Dist2D(SingleWaypoint->GetActorLocation(), GroupMiddlePoint);
			if (Distance < MinDistance) 
			{
				MinDistance = Distance;
				NearestWaypoint = SingleWaypoint;
			}
		}
		if (!NearestWaypoint) 
		{
			
			return; 
		}

		WaypointMap.Add(It.Key(), NearestWaypoint);
	}
}

/**
* Waypoint�� ���� ���¿� ���� ���� ���� Waypoint�� ���� ���Ž�ŵ�ϴ�.
* @param EnemyGroupNumber
*/
void AXCOMGameMode::RenewWaypoint(int8 EnemyGroupNumber) 
{
	AWaypoint* CurrentWaypoint = WaypointMap[EnemyGroupNumber];
	AWaypoint* NextWaypoint = WaypointMap[EnemyGroupNumber]->NextWaypoint;
	AWaypoint* PrevWaypoint = WaypointMap[EnemyGroupNumber]->PrevWaypoint;

	if (CurrentWaypoint->IsForward() && IsValid(NextWaypoint))
	{
		CurrentWaypoint->TurnDirection();
		WaypointMap.Add(EnemyGroupNumber, NextWaypoint);
		UE_LOG(LogTemp, Warning, L"�չ������� ����")
	}
	else if( !CurrentWaypoint->IsForward() && IsValid(PrevWaypoint))
	{
		CurrentWaypoint->TurnDirection();
		WaypointMap.Add(EnemyGroupNumber, PrevWaypoint);
		UE_LOG(LogTemp, Warning, L"�ڹ������� ����")
	}
	else if( !IsValid(NextWaypoint))
	{
		WaypointMap.Add(EnemyGroupNumber, PrevWaypoint);
		UE_LOG(LogTemp, Warning, L"�ڹ������� ����")
	}
	else 
	{
		WaypointMap.Add(EnemyGroupNumber, NextWaypoint);
		UE_LOG(LogTemp, Warning, L"�չ������� ����")
	}
}

/**
* �� AI �׷��� Aggro�� �մϴ�.
* @param EnemyGroupNumber - AI �׷� ��ȣ
*/
void AXCOMGameMode::ChangeEnemyAggro(int8 EnemyGroupNumber) 
{
	TArray<AEnemyUnit*> EnemyGroup = EnemyTeamMap[EnemyGroupNumber];

	for (AEnemyUnit* SingleEnemy : EnemyGroup) 
	{
		AEnemyController* EnemyController = Cast<AEnemyController>(SingleEnemy->GetController());
		SingleEnemy->OnAggo();
		SingleEnemy->SetWalkingState(EWalkingState::Running);
		EnemyController->ChangeBehaviorToCombat();
	}
}

/**
* �÷��̾� ���ֵ��� �߰������� ���ɴϴ�.
*/
FVector AXCOMGameMode::GetPlayerUnitMiddlePoint() 
{
	FVector PlayerUnitsMiddlePoint;
	for (ACustomThirdPerson* SinglePlayerUnit : PlayerCharacters)
	{
		PlayerUnitsMiddlePoint += SinglePlayerUnit->GetActorLocation();
	}
	PlayerUnitsMiddlePoint = PlayerUnitsMiddlePoint / PlayerCharacters.Num();

	return PlayerUnitsMiddlePoint;
}

/**
* Ư�� �̺�Ʈ(���� ǥ��)�� ������ ��ٸ��� ������ �ִ��� Ȯ���մϴ�.
*/
bool AXCOMGameMode::HasEnemyUnitEvent()
{
	if (IsValid(EventUnit)) 
	{
		UE_LOG(LogTemp, Warning, L"^Evnet  Unit �̾�");

		return true;
	}
	else 
	{
		UE_LOG(LogTemp, Warning, L"^Evnet  Unit ����");

		return false;
	}
}

/**
* Ư�� �̺�Ʈ(���� ǥ��) ���� ����� �����ϴ�.
*/
void AXCOMGameMode::ExecuteEnemyEvent()
{
	if (IsValid(EventUnit))
	{
		AEnemyUnit* EventExecutor = Cast<AEnemyUnit>(EventUnit);
		if (EventExecutor) 
		{
			EventExecutor->PlayEvent();
			EventUnit = nullptr;
		}
	}
}

void AXCOMGameMode::RegisterEventActor(AActor* EventActor) 
{
	EventUnit = EventActor;
}

void AXCOMGameMode::CheckTurnAfterEvent() 
{
	CheckTurnOver(TurnBuffer);
}

/**
* ���� ����� �˸��� �˾��� ���ϴ�.
* @param DamagedActor - �������� ���� Actor
* @param Damage - ����Ǵ� ������
* @param State - ���� ����� �˸��� State
*/
void AXCOMGameMode::ShowCombatPopUp(AActor* DamagedActor, float Damage, FloatingWidgetState State)
{
	AXCOMPlayerController* PlayerController = Cast<AXCOMPlayerController>(GetWorld()->GetFirstPlayerController());
	FVector TargetLocation = DamagedActor->GetActorLocation() + FVector(0, 0, 100);
	FVector2D NewWidgetLocation;
	PlayerController->ProjectWorldLocationToScreen(TargetLocation, NewWidgetLocation);
	if (PopUp && !DamagedActor->bHidden)
	{
		PopUp->SetPositionInViewport(NewWidgetLocation);
		PopUp->ShowCombatInfo(Damage, State);
	}
}

/**
* GameMode���� �ش� ������ �����մϴ�.
* @param Unit - ������ Actor
*/
void AXCOMGameMode::UnRegisterUnit(ACustomThirdPerson* Unit)
{
	PlayerCharacters.Remove(Unit);

	if (EnemyCharacters.Find(Unit)) 
	{
		EnemyTurnOrder--;
		EnemyCharacters.RemoveSwap(Unit);
	}
}
