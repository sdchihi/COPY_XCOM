// Fill out your copyright notice in the Description page of Project Settings.

#include "XCOMGameMode.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "CustomThirdPerson.h"
#include "XCOMPlayerController.h"

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
		SingleCharacter->CheckTurnDelegate.BindDynamic(this, &AXCOMGameMode::CheckTurnOver);

		if (SingleCharacter->GetTeamFlag()) 
		{
			PlayerCharacters.Add(SingleCharacter);
		}
		else 
		{
			EnemyCharacters.Add(SingleCharacter);
		}
	}
}

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
		}
		else 
		{
			EnableInput(PlayerController);
			UE_LOG(LogTemp, Warning, L"AI측 턴 오버");
			RestoreTeamActionPoint(PlayerCharacters);
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

void AXCOMGameMode::RestoreTeamActionPoint(TArray<ACustomThirdPerson*>& Characters) 
{
	for (ACustomThirdPerson* SingleCharacter : Characters)
	{
		SingleCharacter->RestoreActionPoint();
	}
}
