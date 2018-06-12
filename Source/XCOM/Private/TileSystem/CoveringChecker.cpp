// Fill out your copyright notice in the Description page of Project Settings.

#include "CoveringChecker.h"
#include "Components/InstancedStaticMeshComponent.h"

// Sets default values
ACoveringChecker::ACoveringChecker()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;
	//InstancedStaticMesh = NewObject<UInstancedStaticMeshComponent>(this);
	//InstancedStaticMesh->RegisterComponent();
	//InstancedStaticMesh->SetFlags(RF_Transactional);
	//this->AddInstanceComponent(InstancedStaticMesh);

}

// Called when the game starts or when spawned
void ACoveringChecker::BeginPlay()
{
	Super::BeginPlay();
	if (MeshToRegister) 
	{
		InstancedStaticMesh->SetStaticMesh(MeshToRegister);
	}
}

// Called every frame
void ACoveringChecker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void ACoveringChecker::AddInstances(TArray<FTransform>& TransformArrray) const 
{
	for (FTransform SingleTransform : TransformArrray) 
	{
		InstancedStaticMesh->AddInstance(SingleTransform);
		UE_LOG(LogTemp, Warning, L"임시 : Instance 생성 디버깅 로그");
	}
}