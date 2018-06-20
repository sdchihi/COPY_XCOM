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
	if (bGenerateFogOfWar) 
	{
		SpawnFogOfWar();
	}
}

/**
* ���� ���� �������� Ȯ���մϴ�.
* @param bIsPlayerTeam - Ȯ���� ���� �÷���
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

	if (bIsEnd)		// �� �ѱ�
	{
		AXCOMPlayerController* PlayerController = Cast<AXCOMPlayerController>(GetWorld()->GetFirstPlayerController());
		if (!PlayerController) { return; }

		if (bIsPlayerTeam) 
		{
			DisableInput(PlayerController);
			UE_LOG(LogTemp, Warning, L"�÷��̾��� �� ����");
			RestoreTeamActionPoint(EnemyCharacters);
			SetEnemysPatrolDirection();
			StartBotActivity();
		}
		else 
		{
			EnableInput(PlayerController);
			UE_LOG(LogTemp, Warning, L"AI�� �� ����");
			RestoreTeamActionPoint(PlayerCharacters);
			EnemyTurnOrder = 0;
		}
		//Todo  AI�� Ȱ����Ű���� Player���� Ȱ��ȭ�ϴ��� ���� �ϳ��� ����
	}
}


void AXCOMGameMode::CheckTurnStateOfOneTeam(TArray<ACustomThirdPerson*>& Characters)
{
	for (auto SingleCharacter : Characters) 
	{
	}
}

/**
* ĳ���͵��� Action point�� ȸ����ŵ�ϴ�.
* @param Characters - ȸ����ų ĳ���͵�
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
	FVector PlayerUnitsMiddlePoint;
	for (ACustomThirdPerson* SinglePlayerUnit : PlayerCharacters)
	{
		PlayerUnitsMiddlePoint += SinglePlayerUnit->GetActorLocation();
	}
	PlayerUnitsMiddlePoint = PlayerUnitsMiddlePoint / PlayerCharacters.Num();

	for (auto It = EnemyTeamMap.CreateConstIterator(); It; ++It)
	{
		FVector EnemyUnitsMiddlePoint;
		bool bSkipLoop = false;
		
		for (AEnemyUnit* SingleEnemyUnit : It.Value())
		{
			if (SingleEnemyUnit->IsAggro()) 
			{
				bSkipLoop = true;
				break;
			}
			FVector EnemyLocation = SingleEnemyUnit->GetActorLocation();
			EnemyUnitsMiddlePoint += EnemyLocation;
		}
		if (bSkipLoop) { continue; }		// ��׷� �����϶� Direction Setting �� �ʿ� ����.

		EnemyUnitsMiddlePoint = EnemyUnitsMiddlePoint / It.Value().Num();
		EDirection DirectionToDetector = GetDirectionFromEnemyGroup(EnemyUnitsMiddlePoint, PlayerUnitsMiddlePoint);
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
