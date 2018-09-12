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
	
	/** 한 액션 포인트로 이동할 수 있는 타일인지 여부 */
	bool bCanMoveWithOneAct;

	/** 유닛이 현재 타일로 이동이 가능한지 여부 */
	bool bCanMove = false;

	class UBoxComponent* GetCollision() { return Collision; };

	void SetTileSize(float Size);
protected:

private:
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* Collision = nullptr;

};
