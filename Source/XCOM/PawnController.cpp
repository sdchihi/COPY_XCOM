// Fill out your copyright notice in the Description page of Project Settings.

#include "PawnController.h"





void APawnController::BeginPlay() {
	Super::BeginPlay();
}



void APawnController::MoveToTargetLocation(FVector TargetLocation) {
	//MoveTo(FAIMoveRequest(TargetLocation));
	MoveToLocation(TargetLocation,0,false, false, false);
}
