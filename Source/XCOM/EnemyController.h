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

	AEnemyController(const class FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	virtual void Possess(class APawn* InPawn) override;

	UFUNCTION(BlueprintCallable)
	void SetNextAction();

	UPROPERTY(EditAnywhere, Category = Behavior)
	class UBehaviorTree* EnemyBehavior;

	void FollowThePath();

	void ShootToPlayerUnit();

	void StopBehaviorTree();

	void StartBehaviorTree();

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

	TArray<FVector> PathToTarget;

	class ATileManager2* TileManager = nullptr;

	TMap<ATile*, FAICommandInfo> GetScoreBoard(TArray<ATile*> MovableTiles);

	EAction NextAction;

	TArray<class ACustomThirdPerson*> GetPlayerCharacters();

	bool CheckMimiumInterval(const FVector TileLocation, const FVector TargetActorLocation);

	bool IsProtectedByCover(const FVector TileLocation, const FVector TargetActorLocation, const TArray<FVector> CoverDirectionArr, OUT bool& bGoodAngle);

	void ScoringByAimingInfo(const FVector TileLocation, TArray<FVector> CoverDirectionArr, OUT int32& ActionScore, OUT FAimingInfo& BestAimingInfo);

	TMap<EDirection, ECoverInfo> MakeCoverDirectionMap(TArray<FVector> CoverDirectionArr);

	EAction DecideActionOnTile(int32 ActionScore);

	void FindBestScoredAction(const TMap<ATile*, FAICommandInfo>& TileScoreBoard);

	void DebugAimingInfo(const FVector TileLocation, const int32 Score);

};