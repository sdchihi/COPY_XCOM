// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Kismet/KismetSystemLibrary.h"
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

	FAimingInfo* AimingInfo = nullptr;

	EAction Action;

	FAICommandInfo() 
	{
		Score = 0;
		AimingInfo = nullptr;
		Action = EAction::None;
	}


	~FAICommandInfo() 
	{
		if (AimingInfo != nullptr) 
		{
			UE_LOG(LogTemp, Warning, L"제거됨")

			delete AimingInfo;
		}
	}

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

	/** 정찰 활동할때 사용하는 행동 트리입니다. */
	UPROPERTY(EditAnywhere, Category = Behavior)
	class UBehaviorTree* PatrolBehavior;

	/** 전투 상황일때 사용하는 행동 트리입니다. */
	UPROPERTY(EditAnywhere, Category = Behavior)
	UBehaviorTree* CombatBehavior;

	/** AI의 다음 행동을 결정합니다. */
	UFUNCTION(BlueprintCallable)
	void SetNextAction();

	/** 정해진 경로를 따라 Unit을 이동시킵니다. */
	void FollowThePath();

	/** 플레이어 유닛을 향해 사격합니다. */
	void ShootToPlayerUnit();

	/** 현재 작동하고있는 행동트리를 정지합니다. */
	void StopBehaviorTree();

	/** 현재 지정된 행동트리를 시작합니다. */
	void StartBehaviorTree();

	/** 현재 지정된 행동트리를 시작합니다. */
	void StartBehaviorTreeFromDefault();

	/** 다음 정찰 위치를 정합니다. */
	void SetNextPatrolLocation();

	void SetPatrolTargetLocation(FVector TargetLocation) 
	{
		UKismetSystemLibrary::DrawDebugPoint(
			GetWorld(),
			TargetLocation,
			20,
			FColor::Green,
			5
		);

		PatrolTargetLocation = TargetLocation; 
	};

	/** 전투 상황의 행동트리로 변경합니다. */
	void ChangeBehaviorToCombat();

	/** 경계 시작을 명령합니다 */
	void OrderStartVigilance();

private:
	/** 현재 사용하는 행동트리 */
	UBehaviorTree* SelectedBehavior = nullptr;

	UPROPERTY(transient)
	class UBlackboardComponent* BlackboardComp;

	UPROPERTY(transient)
	class UBehaviorTreeComponent* BehaviorTreeComp;

	void ChangeBehavior(UBehaviorTree* BehaviorTreeToSet);

	/** 블랙보드와 대응되는 ID */
	int32 NextLocationKeyID;

	/** 블랙보드와 대응되는 ID */
	int32 ActionKeyID;

	/** 블랙보드와 대응되는 ID */
	int32 RemainingMovementKeyID;

	/** 블랙보드와 대응되는 ID */
	int32 FormalProceedKeyID;

	int32 MovementIndex = 0;

	FAimingInfo* AimingInfo;

	TArray<FVector> PathToTarget;

	class ATileManager* TileManager = nullptr;

	EAction NextAction;

	FVector PatrolTargetLocation;

	/** 
	* 이동 가능한 타일들에 대해서 점수를 매깁니다. 
	* @param MovableTiles - 이동 가능한 타일들의 배열
	* @return 맵 자료형의 스코어 보드
	*/
	TMap<ATile*, FAICommandInfo> GetScoreBoard(TArray<ATile*> MovableTiles);

	/**
	* 타일에 점수를 매깁니다.
	* @param TargetTile - 타일
	* @param CoverDirectionArr - 유닛이 엄폐하고 있는 방향 벡터의 배열
	* @param TileScoreBoard - 점수를 매길 스코어 보드
	*/
	void ScoreToTile(ATile* TargetTile, TArray<FVector> CoverDirectionArr, OUT TMap<ATile*, FAICommandInfo>& TileScoreBoard);

	/** 
	* 플레이어측 유닛들의 배열을 얻습니다.
	* @return 플레이어 유닛들의 배열
	*/
	TArray<class ACustomThirdPerson*> GetPlayerCharacters();

	/**
	* 지정된 이격 거리이상 간격을 유지하고 있는지 여부를 확인합니다.
	* @param TileLocation - 타일의 위치
	* @param TargetActorLocation - 타겟 Actor의 위치
	* @return 제한 이격 거리를 유지하고있는지 여부
	*/
	bool CheckMimiumInterval(const FVector TileLocation, const FVector TargetActorLocation);

	/**
	* 유닛이 엄폐를 통해 보호받고 있는지 여부를 확인합니다.
	* @param TileLocation - 타일의 위치
	* @param TargetActorLocation - 타겟 Actor의 위치
	* @param CoverDirectionArr - 유닛이 엄폐하고 있는 방향 벡터의 배열
	* @param bGoodAngle - 좋은 각도로 보호받고 있는지 여부
	* @return 엄폐를 통한 보호 여부
	*/
	bool IsProtectedByCover(const FVector TileLocation, const FVector TargetActorLocation, const TArray<FVector> CoverDirectionArr, OUT bool& bGoodAngle);

	/**
	* 유닛이 엄폐를 통해 보호받고 있는지 여부를 확인합니다.
	* @param TileLocation - 타일의 위치
	* @param TargetActorLocation - 타겟 Actor의 위치
	* @param ActionScore - 유닛이 엄폐하고 있는 방향 벡터의 배열
	* @param BestAimingInfo - 좋은 각도로 보호받고 있는지 여부
	*/
	void ScoringByAimingInfo(const FVector TileLocation, TArray<FVector> CoverDirectionArr, OUT int32& ActionScore, OUT FAimingInfo& BestAimingInfo);
	
	/**
	* 엄폐 방향, 정보를 담은 Map 을 만듭니다.
	* @param CoverDirectionArr - 엄폐 방향 벡터의 배열
	* @return 엄폐 방향, 정보를 담은 Map
	*/
	TMap<EDirection, ECoverInfo> MakeCoverMap(TArray<FVector> CoverDirectionArr);

	/**
	* Score에 따라 적합한 Action을 반환합니다.
	* @param ActionScore - 액션 점수 값
	* @return Action
	*/
	EAction GetActionAccordingToScore(int32 ActionScore) const;

	/**
	* Score에 따라 적합한 Action을 반환합니다.
	* @param ActionScore - 액션 점수 값
	* @return Action
	*/
	void FindBestScoredAction(const TMap<ATile*, FAICommandInfo>& TileScoreBoard);

	void DebugAimingInfo(const FVector TileLocation, const int32 Score);

	/**
	* 목표로 하는 위치가 추적이 가능한지 여부를 확인합니다. ( FOW 와 연계 )
	* @param TargetLocation - 목표로 하는 위치
	* @return 추적 가능 여부 
	*/
	bool CheckTargetLocation(FVector TargetLocation);

	/**
	* 위치에 따라 턴 진행 방식을 제어합니다.
	* @param TargetLocation - 목표로 하는 위치
	*/
	void DecideWayToProceedBasedLocation(FVector TargetLocation);

	/**
	* 제어하던 유닛이 죽었을때 호출됩니다.
	*/
	UFUNCTION()
	void ControlledUnitDead(ACustomThirdPerson* ControlledUnit);

};