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


void ATile::SetDecalRotationYaw(float Yaw) {
	FRotator NewDecalRotation = DecalComponent->GetComponentRotation();
	NewDecalRotation.Yaw = Yaw;
	NewDecalRotation.Roll = 0;

	DecalComponent->SetRelativeRotation(FRotator(90.f,Yaw,0.f));
};
