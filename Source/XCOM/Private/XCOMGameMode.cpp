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
#include "EventExecutor.h"
#include "UMG/Public/Blueprint/UserWidget.h"
#include "FAimingQueue.h"

AXCOMGameMode::AXCOMGameMode()
{
}

AXCOMGameMode::~AXCOMGameMode()
{
	FAimingQueue::Instance().Destroy();
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
	InitializeWaypointMap();

	AXCOMPlayerController* PlayerController = Cast<AXCOMPlayerController>(GetWorld()->GetFirstPlayerController());
	PlayerController->HealthBarVisiblityDelegate.BindDynamic(this, &AXCOMGameMode::SetVisibleAllHealthBar);
	if (bGenerateFogOfWar) 
	{
		SpawnFogOfWar();
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
			DisableInput(PlayerController);
			UE_LOG(LogTemp, Warning, L"플레이어측 턴 오버 - > AI측 턴 시작");
			RestoreTeamActionPoint(EnemyCharacters);
			SetEnemysPatrolLocation();
			StartBotActivity();

		}
		else 
		{
			PlayerController->DisableFocusing();

			EnableInput(PlayerController);
			UE_LOG(LogTemp, Warning, L"AI측 턴 오버");
			RestoreTeamActionPoint(PlayerCharacters);
			EnemyTurnOrder = 0;
		}
		//Todo  AI를 활성시키던지 Player쪽을 활성화하던지 둘중 하나를 수행
	}
}


void AXCOMGameMode::CheckTurnStateOfOneTeam(TArray<ACustomThirdPerson*>& Characters)
{
	for (auto SingleCharacter : Characters) 
	{
	}
}

/**
* 캐릭터들의 Action point를 회복시킵니다.
* @param Characters - 회복시킬 캐릭터들
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


void AXCOMGameMode::StartBotActivity() 
{
	ACustomThirdPerson* EnemyChar = EnemyCharacters[EnemyTurnOrder];
	AEnemyController* EnemyController = EnemyChar ? Cast<AEnemyController>(EnemyChar->GetController()) : nullptr;
	if (!EnemyController) 
	{
		EnemyTurnOrder++;
		StartBotActivity();
	}
	else 
	{
		
		EnemyController->StartBehaviorTreeFromDefault();
		EnemyTurnOrder++;
	}
}

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
		if (bSkipLoop) { continue; }		// 어그로 상태일땐 Direction Setting 할 필요 없음.

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

void AXCOMGameMode::ExecuteEnemyEvent()
{
	if (IsValid(EventUnit))
	{
		AEnemyUnit* EventExecutor = Cast<AEnemyUnit>(EventUnit);
		if (EventExecutor) 
		{
			UE_LOG(LogTemp, Warning, L"^ Mode 에서 이벤트실행 명령떨어짐")

			EventExecutor->PlayEvent();
			EventUnit = nullptr;
		}
	}
	else 
	{
		UE_LOG(LogTemp, Warning, L"^유효하지않음")
	}
}

void AXCOMGameMode::RegisterEventActor(AActor* EventActor) 
{
	UE_LOG(LogTemp, Warning, L"^등록됨")

	EventUnit = EventActor;
}

void AXCOMGameMode::CheckTurnAfterEvent() 
{
	UE_LOG(LogTemp, Warning, L"^이것은 완료후 Checkturn Event")

	CheckTurnOver(TurnBuffer);
}


void AXCOMGameMode::ShowCombatPopUp(AActor* DamagedActor, float Damage) 
{
}
