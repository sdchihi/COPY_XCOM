// Fill out your copyright notice in the Description page of Project Settings.

#include "PathIndicator.h"
#include "Classes/Components/InstancedStaticMeshComponent.h"


APathIndicator::APathIndicator()
{
	PrimaryActorTick.bCanEverTick = false;
	DirectionMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(FName(L"DirectionMesh"));
}

void APathIndicator::BeginPlay()
{
	Super::BeginPlay();
}

void APathIndicator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APathIndicator::ClearAllTile() 
{
	DirectionMesh->ClearInstances();
}


void APathIndicator::IndicateDirection(TArray<FTransform> PathTransform) 
{
	ClearAllTile();
	for (FTransform DirectionTransform : PathTransform)
	{
		DirectionMesh->AddInstance(DirectionTransform);
	}
}
