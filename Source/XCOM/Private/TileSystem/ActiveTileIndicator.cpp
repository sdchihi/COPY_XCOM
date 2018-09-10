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

/**
* 활성화된 Tile들 위에 Mesh를 생성합니다.
* @param CloseTileTransArray - 가까운 타일들의 Transform 목록
* @param DistantTileTransArray - 먼 타일들의 Transform 목록
*/
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

void AActiveTileIndicator::SetTileVisibility(bool Visible) 
{
	DistantTileMsh->SetVisibility(Visible);
	CloseTileMesh->SetVisibility(Visible);
}
