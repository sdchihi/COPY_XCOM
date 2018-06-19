// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerDetector.generated.h"

UCLASS()
class XCOM_API APlayerDetector : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlayerDetector();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	float SumOfDistancesToPlayerUnit(TArray<AActor*>& PlayerUnitArray) const;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
