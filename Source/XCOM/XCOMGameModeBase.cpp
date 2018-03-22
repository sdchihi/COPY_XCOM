// Fill out your copyright notice in the Description page of Project Settings.

#include "XCOMGameModeBase.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "CustomThirdPerson.h"

AXCOMGameModeBase::AXCOMGameModeBase() 
{
	
	
}



void AXCOMGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACustomThirdPerson::StaticClass(), FoundActors);

	for (auto SingleActor : FoundActors)
	{
		ACustomThirdPerson* SingleCharacter = Cast<ACustomThirdPerson>(SingleActor);
		//if (SingleCharacter->GetTeamFlag()) 
		//{
		//	PlayerCharacters.Add(SingleCharacter);
		//}
		//else 
		//{
		//	EnemyCharacters.Add(SingleCharacter);
		//}
	}
}


void AXCOMGameModeBase::CheckTurnOver(const bool bIsPlayerTeam)
{
	if (bIsPlayerTeam) 
	{
		
	}
	else 
	{
	}
}

void AXCOMGameModeBase::CheckTurnStateOfOneTeam(TArray<ACustomThirdPerson>& Characters)
{
	/*for (auto SingleCharacter : Characters) 
	{
	}*/
}
