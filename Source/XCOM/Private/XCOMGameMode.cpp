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
* 팀의 턴의 끝났는지 확인합니다.
* @param bIsPlayerTeam - 확인할 팀의 플래그
*/
void AXCOMGameMode::CheckTurnOver(const bool bIsPlayerTeam)
{
	UE_LOG(LogTemp, Warning, L"^턴오버 확인 호출됨");

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

	if (bIsEnd)		// 턴 넘김
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
* Player측의 턴을 종료합니다.
*/
void AXCOMGameMode::PlayerTurnOver() 
{
	AXCOMPlayerController* PlayerController = Cast<AXCOMPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PlayerController) { return; }
	RestoreTeamActionPoint(EnemyCharacters);

	DisableInput(PlayerController);
	PlayerController->FinishPlayerTurn();
	UE_LOG(LogTemp, Warning, L"플레이어측 턴 오버 - > AI측 턴 시작");
	SetEnemysPatrolLocation();
	EnemyTurnOrder = 0;
	StartBotActivity();
}

/**
* AI측의 턴을 종료합니다.
*/
void AXCOMGameMode::EnemyTurnOver()
{
	AXCOMPlayerController* PlayerController = Cast<AXCOMPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PlayerController) { return; }

	PlayerController->DisableFocusing();
	EnableInput(PlayerController);
	UE_LOG(LogTemp, Warning, L"AI측 턴 오버");
	RestoreTeamActionPoint(PlayerCharacters);
	PlayerController->FocusNextAvailablePlayerUnit(true);
}

/**
* 캐릭터들의 Action point를 회복시킵니다.
* @param Characters - 회복시킬 캐릭터들 배열
*/
void AXCOMGameMode::RestoreTeamActionPoint(TArray<ACustomThirdPerson*>& Characters) 
{
	for (ACustomThirdPerson* SingleCharacter : Characters)
	{
		SingleCharacter->RestoreActionPoint();
	}
}

/**
* 캐릭터들의 Health bar의 가시성을 변경합니다.
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
* FOW(전장의 안개)를 생성합니다.
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
* Player Unit 혹은 Ai Unit 배열을 얻어옵니다.
* @param bTeam - 플레이어 / AI
* @return 한쪽 팀의 유닛 배열
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
* 한 AI Unit의 활동을 시작합니다.
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
* AI Unit의 정찰 위치를 결정합니다.
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
* Enemy Group 별로 정찰을 하게될 Waypoint를 Map으로 구성합니다
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
* Waypoint의 연결 상태에 따라 다음 진행 Waypoint로 맵을 갱신시킵니다.
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
		UE_LOG(LogTemp, Warning, L"앞방향으로 진행")
	}
	else if( !CurrentWaypoint->IsForward() && IsValid(PrevWaypoint))
	{
		CurrentWaypoint->TurnDirection();
		WaypointMap.Add(EnemyGroupNumber, PrevWaypoint);
		UE_LOG(LogTemp, Warning, L"뒤방향으로 진행")
	}
	else if( !IsValid(NextWaypoint))
	{
		WaypointMap.Add(EnemyGroupNumber, PrevWaypoint);
		UE_LOG(LogTemp, Warning, L"뒤방향으로 진행")
	}
	else 
	{
		WaypointMap.Add(EnemyGroupNumber, NextWaypoint);
		UE_LOG(LogTemp, Warning, L"앞방향으로 진행")
	}
}

/**
* 한 AI 그룹의 Aggro를 켭니다.
* @param EnemyGroupNumber - AI 그룹 번호
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
* 플레이어 유닛들의 중간지점을 얻어옵니다.
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
* 특정 이벤트(감정 표현)의 실행을 기다리는 유닛이 있는지 확인합니다.
*/
bool AXCOMGameMode::HasEnemyUnitEvent()
{
	if (IsValid(EventUnit)) 
	{
		UE_LOG(LogTemp, Warning, L"^Evnet  Unit 이씀");

		return true;
	}
	else 
	{
		UE_LOG(LogTemp, Warning, L"^Evnet  Unit 없음");

		return false;
	}
}

/**
* 특정 이벤트(감정 표현) 실행 명령을 내립니다.
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
* 전투 결과를 알리는 팝업을 띄웁니다.
* @param DamagedActor - 데미지를 입은 Actor
* @param Damage - 적용되는 데미지
* @param State - 전투 결과를 알리는 State
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
* GameMode에서 해당 유닛을 해지합니다.
* @param Unit - 해지할 Actor
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
