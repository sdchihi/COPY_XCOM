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
	struct FAimingInfo AimingInfo;

	class ATileManager2* TileManager = nullptr;
	
	TMap<ATile*, int32> AEnemyController::GetScoreBoard(TArray<ATile*> MovableTiles);

	EAction NextAction;

	TArray<class ACustomThirdPerson*> GetPlayerCharacters();

	bool CheckMimiumInterval(const FVector TileLocation, const FVector TargetActorLocation);

	bool IsProtectedByCover(const FVector TileLocation, const FVector TargetActorLocation, const TArray<FVector> CoverDirectionArr)


};
