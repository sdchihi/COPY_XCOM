// Fill out your copyright notice in the Description page of Project Settings.

#include "PawnController.h"
#include "CustomThirdPerson.h"




void APawnController::BeginPlay() {
	Super::BeginPlay();
}



void APawnController::MoveToTargetLocation(FVector TargetLocation) {
	MoveToLocation(TargetLocation,0,false, false, false);
}

void APawnController::BindVigilanceEvent(const TArray<ACustomThirdPerson*> OppositeTeamMember)
{
	ACustomThirdPerson* ControlledPawn =Cast<ACustomThirdPerson>(GetPawn());

	if (ControlledPawn) 
	{
		for (ACustomThirdPerson* SingleEnemyCharacter : OppositeTeamMember)
		{
			SingleEnemyCharacter->UnprotectedMovingDelegate.AddUniqueDynamic(this, &APawnController::WatchOut);
		}
	}
	
}


void APawnController::WatchOut(const FVector TargetLocation) 
{

	ACustomThirdPerson* ControlledPawn = Cast<ACustomThirdPerson>(GetPawn());
	if (ControlledPawn)
	{
		ControlledPawn->InVigilance(TargetLocation);
	}
}
