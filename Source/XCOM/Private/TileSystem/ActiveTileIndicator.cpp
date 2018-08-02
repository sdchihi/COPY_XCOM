// Fill out your copyright notice in the Description page of Project Settings.

#include "ActiveTileIndicator.h"
#include "Classes/Components/InstancedStaticMeshComponent.h"

// Sets default values
AActiveTileIndicator::AActiveTileIndicator()
{
	PrimaryActorTick.bCanEverTick = false;
	CloseTileMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(FName(L"CloseTileMesh"));
	DistantTileMsh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(FName(L"DistantTileMsh"));

}

void AActiveTileIndicator::BeginPlay()
{
	Super::BeginPlay();
	
}

void AActiveTileIndicator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AActiveTileIndicator::ClearAllTile()
{
	CloseTileMesh->ClearInstances();
	DistantTileMsh->ClearInstances();
}

void AActiveTileIndicator::IndicateActiveTiles(TArray<FTransform> CloseTileTransArray, TArray<FTransform> DistantTileTransArray)
{
	ClearAllTile();
	for (FTransform DistantTileTransform : DistantTileTransArray)
	{
		DistantTileMsh->AddInstance(DistantTileTransform);
	}

	for (FTransform CloseTileTransform : CloseTileTransArray)
	{
		CloseTileMesh->AddInstance(CloseTileTransform);
	}
}


