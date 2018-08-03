// Fill out your copyright notice in the Description page of Project Settings.

#include "Tile.h"
#include "Classes/Components/DecalComponent.h"
#include "Classes/Components/BoxComponent.h"


ATile::ATile() 
{
	
	Collision = CreateDefaultSubobject<UBoxComponent>(FName(L"BoxCollision"));
	
}

void ATile::BeginPlay()
{
	PrimaryActorTick.bCanEverTick = false;

	Super::BeginPlay();
};

void ATile::SetTileSize(float Size) 
{
	Collision->SetBoxExtent(FVector(Size, Size, 50));
}
