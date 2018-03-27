// Fill out your copyright notice in the Description page of Project Settings.

#include "XCOMGameMode.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "CustomThirdPerson.h"
#include "XCOMPlayerController.h"

AXCOMGameMode::AXCOMGameMode()
{
	UE_LOG(LogTemp, Warning, L"����Ȯ�� 4555");

}



void AXCOMGameMode::BeginPlay()
{
	UE_LOG(LogTemp, Warning, L"����Ȯ�� 43");

	Super::BeginPlay();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACustomThirdPerson::StaticClass(), FoundActors);
	UE_LOG(LogTemp, Warning, L"����Ȯ�� 4");

	for (auto SingleActor : FoundActors)
	{
		ACustomThirdPerson* SingleCharacter = Cast<ACustomThirdPerson>(SingleActor);
		SingleCharacter->CheckTurnDelegate.BindDynamic(this, &AXCOMGameMode::CheckTurnOver);
		UE_LOG(LogTemp, Warning, L"����Ȯ�� 5");

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

	if (bIsEnd)		// �� �ѱ�
	{
		AXCOMPlayerController* PlayerController = Cast<AXCOMPlayerController>(GetWorld()->GetFirstPlayerController());
		if (!PlayerController) { return; }

		if (bIsPlayerTeam) 
		{
			DisableInput(PlayerController);
			UE_LOG(LogTemp, Warning, L"�÷��̾��� �� ����");
			RestoreTeamActionPoint(EnemyCharacters);
		}
		else 
		{
			EnableInput(PlayerController);
			UE_LOG(LogTemp, Warning, L"AI�� �� ����");
			RestoreTeamActionPoint(PlayerCharacters);
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

void AXCOMGameMode::RestoreTeamActionPoint(TArray<ACustomThirdPerson*>& Characters) 
{
	for (ACustomThirdPerson* SingleCharacter : Characters)
	{
		SingleCharacter->RestoreActionPoint();
	}
}
