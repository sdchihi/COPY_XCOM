// Fill out your copyright notice in the Description page of Project Settings.

#include "XCOMGameMode.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "CustomThirdPerson.h"
#include "XCOMPlayerController.h"
#include "FogOfWarManager.h"
#include "EnemyController.h"

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
			EnemyCharacters.Add(SingleCharacter);
		}
	}
	AXCOMPlayerController* PlayerController = Cast<AXCOMPlayerController>(GetWorld()->GetFirstPlayerController());
	PlayerController->HealthBarVisiblityDelegate.BindDynamic(this, &AXCOMGameMode::SetVisibleAllHealthBar);

	SpawnFogOfWar();
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