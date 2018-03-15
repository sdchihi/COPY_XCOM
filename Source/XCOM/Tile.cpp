// Fill out your copyright notice in the Description page of Project Settings.

#include "Tile.h"
//#include "Classes/Components/StaticMeshComponent.h"
#include "Classes/Components/DecalComponent.h"





ATile::ATile() {
}

void ATile::BeginPlay() {

	Super::BeginPlay();

	DecalComponent = FindComponentByClass<UDecalComponent>();
	TileMesh = Cast<UStaticMeshComponent>(RootComponent);
};


void ATile::SetDecalVisibility(const bool Visibility) {
	DecalComponent->SetVisibility(Visibility);
}


void ATile::SetDecalRotationYaw(const float Yaw) {
	FRotator NewDecalRotation = DecalComponent->GetComponentRotation();
	NewDecalRotation.Yaw = Yaw;
	NewDecalRotation.Roll = 0;

	DecalComponent->SetRelativeRotation(FRotator(90.f,Yaw,0.f));
};

bool ATile::GetTileVisibility() {
	return TileMesh->IsVisible();
}