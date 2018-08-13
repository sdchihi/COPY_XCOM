// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ActiveTileIndicator.generated.h"

UCLASS()
class XCOM_API AActiveTileIndicator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AActiveTileIndicator();

	UPROPERTY(VisibleAnywhere)
	class UInstancedStaticMeshComponent* CloseTileMesh;

	UPROPERTY(VisibleAnywhere)
	class UInstancedStaticMeshComponent* DistantTileMsh;

	void ClearAllTile();

	void IndicateActiveTiles(TArray<FTransform> CloseTileTransArray, TArray<FTransform> DistantTileTransArray);

	void SetTileVisibility(bool Visible);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

};
