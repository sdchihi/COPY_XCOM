// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyController.generated.h"

class ATile;
enum class EAction : uint8;

/**
 * 
 */
UCLASS()
class XCOM_API AEnemyController : public AAIController
{
	GENERATED_BODY()
	
	
public:
	virtual void BeginPlay() override;

	void SetNextAction();


private:

	class ATileManager2* TileManager = nullptr;
	
	int32 ScoringOnTile(ATile* AvailableTile);

	EAction NextAction;

	TArray<class ACustomThirdPerson*> GetPlayerCharacters();


};
