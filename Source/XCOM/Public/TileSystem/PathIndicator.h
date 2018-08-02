// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PathIndicator.generated.h"

UCLASS()
class XCOM_API APathIndicator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APathIndicator();


	UPROPERTY(VisibleAnywhere)
	class UInstancedStaticMeshComponent* DirectionMesh;

	void ClearAllTile();

	void IndicateDirection(TArray<FTransform> PathTransform);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
