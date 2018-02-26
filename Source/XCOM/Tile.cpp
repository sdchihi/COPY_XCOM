// Fill out your copyright notice in the Description page of Project Settings.

#include "Tile.h"
#include "Classes/Components/DecalComponent.h"





ATile::ATile() {
}

void ATile::BeginPlay() {

	Super::BeginPlay();

	DecalComponent = FindComponentByClass<UDecalComponent>();
	
};


void ATile::SetDecalVisibility(bool Visibility) {
	DecalComponent->SetVisibility(Visibility);
}



void ATile::SetDecalDirection(float NewZAngle) {
	FRotator NewRotation = DecalComponent->GetComponentRotation();
	NewRotation.Yaw = NewZAngle;
	DecalComponent->SetRelativeRotation(NewRotation);
}