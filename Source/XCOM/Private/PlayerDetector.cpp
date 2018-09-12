// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerDetector.h"


// Sets default values
APlayerDetector::APlayerDetector()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void APlayerDetector::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlayerDetector::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

float APlayerDetector::SumOfDistancesToPlayerUnit(TArray<AActor*>& PlayerUnitArray) const
{
	float SumOfDistances = 0;

	for (AActor* PlayerUnit : PlayerUnitArray) 
	{
		float DistanceToUnit = FVector::Dist2D(PlayerUnit->GetActorLocation(), GetActorLocation());
		SumOfDistances += DistanceToUnit;
	}

	return SumOfDistances;
}
