// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyController.generated.h"

class ATile;
enum class EAction : uint8;
enum class EDirection : uint8;
enum class ECoverInfo : uint8;
struct FAimingInfo;

USTRUCT()
struct FAICommandInfo
{
	GENERATED_BODY()

	int32 eee;

	int32 Score;

	FAimingInfo* AimingInfo;

	EAction Action;

	/*FAICommandInfo(int32 TotalScore, FAimingInfo* pAimingInfo, EAction NextAction)
	{
		Score = TotalScore;
		AimingInfo = pAimingInfo;
		Action = NextAction;
	};*/
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


private:
	
	FAimingInfo* AimingInfo;

	class ATileManager2* TileManager = nullptr;
	
	TMap<ATile*, FAICommandInfo> GetScoreBoard(TArray<ATile*> MovableTiles);

	EAction NextAction;

	TArray<class ACustomThirdPerson*> GetPlayerCharacters();

	bool CheckMimiumInterval(const FVector TileLocation, const FVector TargetActorLocation);

	bool IsProtectedByCover(const FVector TileLocation, const FVector TargetActorLocation, const TArray<FVector> CoverDirectionArr);


	void ScoringByAimingInfo(TArray<FVector> CoverDirectionArr, OUT int32& ActionScore, OUT FAimingInfo& BestAimingInfo);

	TMap<EDirection, ECoverInfo> MakeCoverDirectionMap(TArray<FVector> CoverDirectionArr);
	
	EAction DecideActionOnTile(int32 ActionScore);

	void FindBestScoredAction(const TMap<ATile*, FAICommandInfo> TileScoreBoard);

};
