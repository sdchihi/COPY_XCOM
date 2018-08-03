// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tile.generated.h"

/**
 * 
 */
UCLASS()
class XCOM_API ATile : public AActor
{
	GENERATED_BODY()
	
public:
	ATile();

	virtual void BeginPlay() override;
	
	bool bCanMoveWithOneAct;

	bool bCanMove = false;

	class UBoxComponent* GetCollision() { return Collision; };

	void SetTileSize(float Size);
protected:

private:
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* Collision = nullptr;

};
