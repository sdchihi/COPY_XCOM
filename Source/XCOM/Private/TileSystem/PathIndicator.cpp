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


/**
* 이동할 방향들을 타일위에 표시합니다.
* @param PathTransform - 이동 방향을 표시할 Transform 배열
*/
void APathIndicator::IndicateDirection(TArray<FTransform> PathTransform) 
{
	ClearAllTile();
	for (FTransform DirectionTransform : PathTransform)
	{
		DirectionMesh->AddInstance(DirectionTransform);
	}
}
