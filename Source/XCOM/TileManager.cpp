// Fill out your copyright notice in the Description page of Project Settings.

#include "TileManager.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Classes/GameFramework/Actor.h"

// Sets default values
ATileManager::ATileManager()
{
	PrimaryActorTick.bCanEverTick = false;

	RootMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("Root Component"));
}

void ATileManager::BeginPlay()
{
	Super::BeginPlay();

	//Path arr �ʱ�ȭ
	/*for (int i = 0; i < (x*y); i++) {
		Path DefaultPathValue;
		DefaultPathValue.bWall = false;
		DefaultPathValue.Cost = 0;
		
		PathArr.Add(DefaultPathValue);
	}*/

	RootMesh->SetWorldScale3D(FVector(x*TileSize, y*TileSize, 1));

	// ��ġ�� ���͵� Ȯ��
	RootMesh->GetOverlappingActors(OverlappedActors);
	for (int i = 0; i < OverlappedActors.Num(); i++) {
		OverlappedActors[i]->GetActorLocation();
	}

}

void ATileManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

//TODO �迭�� ��� ���͵� ��ġ Array
