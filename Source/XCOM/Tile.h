// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Tile.generated.h"

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

//
//UENUM(BlueprintType)
//enum class EDecalShape : uint8 {
//	Vertical,
//	Horizontal,
//	RisingDiagonal,
//	DescendingDiagonal,
//	EndPointFromEast,
//	EndPointFromWest,
//	EndPointFromSouth,
//	EndPointFromNorth,
//	EndPointFromNorthEast,
//	EndPointFromNorthWest,
//	EndPointFromSouthEast,
//	EndPointFromSouthWest,
//	None
//};
//


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

	void SetDecalVisibility(bool Visibility);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void SetDecalDirection(float NewZAngle);

protected:
	


private:

	UDecalComponent* DecalComponent = nullptr;

};
