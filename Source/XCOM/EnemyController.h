// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyController.generated.h"

class ATile;
class Path;
enum class EAction : uint8;
enum class EDirection : uint8;
enum class ECoverInfo : uint8;
struct FAimingInfo;

USTRUCT()
struct FAICommandInfo
{
	GENERATED_BODY()

	int32 Score;

	FAimingInfo* AimingInfo;

	EAction Action;
};

/**
 * 
 */
UCLASS()
class XCOM_API AEnemyController : public AAIController
{
	GENERATED_BODY()


public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void SetNextAction();

	void RenewNextLocation();


private:

	UPROPERTY(transient)
	class UBlackboardComponent* BlackboardComp;

	UPROPERTY(transient)
	class UBehaviorTreeComponent* BehaviorTreeComp;

	int32 NextLocationKeyID;

	int32 ActionKeyID;

	int32 RemainingMovementKeyID;

	int32 MovementIndex = 0;

	FAimingInfo* AimingInfo;

	TArray<int32> PathToTarget;

	class ATileManager2* TileManager = nullptr;

	TMap<ATile*, FAICommandInfo> GetScoreBoard(TArray<ATile*> MovableTiles);

	EAction NextAction;

	TArray<class ACustomThirdPerson*> GetPlayerCharacters();

	bool CheckMimiumInterval(const FVector TileLocation, const FVector TargetActorLocation);

	bool IsProtectedByCover(const FVector TileLocation, const FVector TargetActorLocation, const TArray<FVector> CoverDirectionArr, OUT bool& bGoodAngle);

	void ScoringByAimingInfo(const FVector TileLocation, TArray<FVector> CoverDirectionArr, OUT int32& ActionScore, OUT FAimingInfo& BestAimingInfo);

	TMap<EDirection, ECoverInfo> MakeCoverDirectionMap(TArray<FVector> CoverDirectionArr);

	EAction DecideActionOnTile(int32 ActionScore);

	void FindBestScoredAction(const TMap<ATile*, FAICommandInfo> TileScoreBoard);

	void DebugAimingInfo(const FVector TileLocation, const int32 Score);


	void ShootToPlayerUnit();

};