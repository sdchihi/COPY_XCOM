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
//	FRotator NewDecalRotation = DecalComponent->GetRelativeTransform().Rotator();
	FRotator NewDecalRotation = DecalComponent->GetComponentRotation();

	NewDecalRotation.SetComponentForAxis(EAxis::Z, Yaw);
	
	UE_LOG(LogTemp, Warning, L"Decal Rot : %s", *DecalComponent->GetRelativeTransform().Rotator().ToString());


	//DecalComponent->SetRelativeRotation(NewDecalRotation);

	DecalComponent->SetWorldRotation(NewDecalRotation);
};
