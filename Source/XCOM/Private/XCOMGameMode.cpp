// Fill out your copyright notice in the Description page of Project Settings.

#include "XCOMGameMode.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "EnemyUnit.h"
#include "CustomThirdPerson.h"
#include "XCOMPlayerController.h"
#include "FogOfWarManager.h"
#include "EnemyController.h"
#include "PlayerDetector.h"

AXCOMGameMode::AXCOMGameMode()
{
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

		if (SingleCharacter->GetTeamFlag()) 
		{
			PlayerCharacters.Add(SingleCharacter);
		}
		else 
		{
			AEnemyUnit* SingleEnemy = Cast<AEnemyUnit>(SingleCharacter);
			
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
	FoundActors.Empty();
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerDetector::StaticClass(), FoundActors);
	for (auto SingleDetector : FoundActors)
	{
		PlayerDetectors.Add(Cast<APlayerDetector>(SingleDetector));
	}

	AXCOMPlayerController* PlayerController = Cast<AXCOMPlayerController>(GetWorld()->GetFirstPlayerController());
	PlayerController->HealthBarVisiblityDelegate.BindDynamic(this, &AXCOMGameMode::SetVisibleAllHealthBar);

	SpawnFogOfWar();
}

/**
* 팀의 턴의 끝났는지 확인합니다.
* @param bIsPlayerTeam - 확인할 팀의 플래그
*/
void AXCOMGameMode::CheckTurnOver(const bool bIsPlayerTeam)
{
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
			UE_LOG(LogTemp, Warning, L"플레이어측 턴 오버");
			RestoreTeamActionPoint(EnemyCharacters);
			StartBotActivity();
		}
		else 
		{
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

	AFogOfWarManager* FogOfWar = GetWorld()->SpawnActor<AFogOfWarManager>(
		FogOfWarBP,
		FVector(0, 0, 0),
		FRotator(0, 0, 0)
		);

	for (ACustomThirdPerson* SinglePlayerChar : PlayerCharacters)
	{
		FogOfWar->RegisterFowActor(SinglePlayerChar);
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

void AXCOMGameMode::SetEnemysPatrolDirection()
{
	for (auto It = EnemyTeamMap.CreateConstIterator(); It; ++It)
	{
		float MinSumOfDistance = MAX_FLT;
		FVector ValidDectectorLocation;
		FVector MiddlePoint;
		for (APlayerDetector* Detector : PlayerDetectors) 
		{
			float SumOfDistace = 0;
			for (AEnemyUnit* SingleEnemyUnit : It.Value())
			{
				FVector EnemyLocation = SingleEnemyUnit->GetActorLocation();
				FVector DetectorLocation = Detector->GetActorLocation();
				float Distance = FVector::Dist2D(EnemyLocation, DetectorLocation);
				MiddlePoint += EnemyLocation;
				SumOfDistace += Distance;
			}
			if (SumOfDistace < MinSumOfDistance) 
			{
				MinSumOfDistance = SumOfDistace;
				ValidDectectorLocation = Detector->GetActorLocation();
			}
		}

		MiddlePoint = MiddlePoint / It.Value().Num();
		EDirection DirectionToDetector = GetDirectionFromEnemyGroup(MiddlePoint, ValidDectectorLocation);
		for (AEnemyUnit* SingleEnemyUnit : It.Value()) 
		{
			AEnemyController* EnemyUnitController = Cast<AEnemyController>(SingleEnemyUnit->GetController());
			EnemyUnitController->SetPatrolDirection(DirectionToDetector);
		}
	}
}

EDirection AXCOMGameMode::GetDirectionFromEnemyGroup(FVector GroupMiddlePoint, FVector DetectorLocation) const 
{
	FVector DirectionToDetector  = DetectorLocation - GroupMiddlePoint;
	float AbsoluteValueX = FMath::Abs(DirectionToDetector.X);
	float AbsoluteValueY = FMath::Abs(DirectionToDetector.Y);
	if (AbsoluteValueX < AbsoluteValueY) 
	{
		if (DirectionToDetector.Y < 0)	
		{
			return EDirection::South;
		}
		else  
		{
			return EDirection::North;
		}
	}
	else
	{
		if (DirectionToDetector.X < 0)	
		{
			return EDirection::East;
		}
		else  
		{
			return EDirection::West;
		}
	}
	return EDirection::None;
}
