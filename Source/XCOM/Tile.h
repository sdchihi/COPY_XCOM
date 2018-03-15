// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Tile.generated.h"

//class UStaticMeshComponent;
class UDecalComponent;

UENUM(BlueprintType)
enum class EDecalShape : uint8 {
	West ,
	East ,
	North ,
	South ,
	NorthWest,
	NorthEast,
	SouthWest,
	SouthEast,
	None
};


/**
 * 
 */
UCLASS()
class XCOM_API ATile : public AStaticMeshActor
{
	GENERATED_BODY()
	
public:
	ATile();

	
	EDecalShape DecalShape = EDecalShape::None;

	void SetDecalVisibility(const bool Visibility);

	void SetDecalRotationYaw(const float Yaw);

	bool GetTileVisibility();

	virtual void BeginPlay() override;

protected:
	


private:

	UDecalComponent* DecalComponent = nullptr;

	UStaticMeshComponent* TileMesh = nullptr;
};
