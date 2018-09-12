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
			UE_LOG(LogTemp, Warning, L"���ŵ�")

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

	/** ���� Ȱ���Ҷ� ����ϴ� �ൿ Ʈ���Դϴ�. */
	UPROPERTY(EditAnywhere, Category = Behavior)
	class UBehaviorTree* PatrolBehavior;

	/** ���� ��Ȳ�϶� ����ϴ� �ൿ Ʈ���Դϴ�. */
	UPROPERTY(EditAnywhere, Category = Behavior)
	UBehaviorTree* CombatBehavior;

	/** AI�� ���� �ൿ�� �����մϴ�. */
	UFUNCTION(BlueprintCallable)
	void SetNextAction();

	/** ������ ��θ� ���� Unit�� �̵���ŵ�ϴ�. */
	void FollowThePath();

	/** �÷��̾� ������ ���� ����մϴ�. */
	void ShootToPlayerUnit();

	/** ���� �۵��ϰ��ִ� �ൿƮ���� �����մϴ�. */
	void StopBehaviorTree();

	/** ���� ������ �ൿƮ���� �����մϴ�. */
	void StartBehaviorTree();

	/** ���� ������ �ൿƮ���� �����մϴ�. */
	void StartBehaviorTreeFromDefault();

	/** ���� ���� ��ġ�� ���մϴ�. */
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

	/** ���� ��Ȳ�� �ൿƮ���� �����մϴ�. */
	void ChangeBehaviorToCombat();

	/** ��� ������ ����մϴ� */
	void OrderStartVigilance();

private:
	/** ���� ����ϴ� �ൿƮ�� */
	UBehaviorTree* SelectedBehavior = nullptr;

	UPROPERTY(transient)
	class UBlackboardComponent* BlackboardComp;

	UPROPERTY(transient)
	class UBehaviorTreeComponent* BehaviorTreeComp;

	void ChangeBehavior(UBehaviorTree* BehaviorTreeToSet);

	/** ������� �����Ǵ� ID */
	int32 NextLocationKeyID;

	/** ������� �����Ǵ� ID */
	int32 ActionKeyID;

	/** ������� �����Ǵ� ID */
	int32 RemainingMovementKeyID;

	/** ������� �����Ǵ� ID */
	int32 FormalProceedKeyID;

	int32 MovementIndex = 0;

	FAimingInfo* AimingInfo;

	TArray<FVector> PathToTarget;

	class ATileManager* TileManager = nullptr;

	EAction NextAction;

	FVector PatrolTargetLocation;

	/** 
	* �̵� ������ Ÿ�ϵ鿡 ���ؼ� ������ �ű�ϴ�. 
	* @param MovableTiles - �̵� ������ Ÿ�ϵ��� �迭
	* @return �� �ڷ����� ���ھ� ����
	*/
	TMap<ATile*, FAICommandInfo> GetScoreBoard(TArray<ATile*> MovableTiles);

	/**
	* Ÿ�Ͽ� ������ �ű�ϴ�.
	* @param TargetTile - Ÿ��
	* @param CoverDirectionArr - ������ �����ϰ� �ִ� ���� ������ �迭
	* @param TileScoreBoard - ������ �ű� ���ھ� ����
	*/
	void ScoreToTile(ATile* TargetTile, TArray<FVector> CoverDirectionArr, OUT TMap<ATile*, FAICommandInfo>& TileScoreBoard);

	/** 
	* �÷��̾��� ���ֵ��� �迭�� ����ϴ�.
	* @return �÷��̾� ���ֵ��� �迭
	*/
	TArray<class ACustomThirdPerson*> GetPlayerCharacters();

	/**
	* ������ �̰� �Ÿ��̻� ������ �����ϰ� �ִ��� ���θ� Ȯ���մϴ�.
	* @param TileLocation - Ÿ���� ��ġ
	* @param TargetActorLocation - Ÿ�� Actor�� ��ġ
	* @return ���� �̰� �Ÿ��� �����ϰ��ִ��� ����
	*/
	bool CheckMimiumInterval(const FVector TileLocation, const FVector TargetActorLocation);

	/**
	* ������ ���� ���� ��ȣ�ް� �ִ��� ���θ� Ȯ���մϴ�.
	* @param TileLocation - Ÿ���� ��ġ
	* @param TargetActorLocation - Ÿ�� Actor�� ��ġ
	* @param CoverDirectionArr - ������ �����ϰ� �ִ� ���� ������ �迭
	* @param bGoodAngle - ���� ������ ��ȣ�ް� �ִ��� ����
	* @return ���� ���� ��ȣ ����
	*/
	bool IsProtectedByCover(const FVector TileLocation, const FVector TargetActorLocation, const TArray<FVector> CoverDirectionArr, OUT bool& bGoodAngle);

	/**
	* ������ ���� ���� ��ȣ�ް� �ִ��� ���θ� Ȯ���մϴ�.
	* @param TileLocation - Ÿ���� ��ġ
	* @param TargetActorLocation - Ÿ�� Actor�� ��ġ
	* @param ActionScore - ������ �����ϰ� �ִ� ���� ������ �迭
	* @param BestAimingInfo - ���� ������ ��ȣ�ް� �ִ��� ����
	*/
	void ScoringByAimingInfo(const FVector TileLocation, TArray<FVector> CoverDirectionArr, OUT int32& ActionScore, OUT FAimingInfo& BestAimingInfo);
	
	/**
	* ���� ����, ������ ���� Map �� ����ϴ�.
	* @param CoverDirectionArr - ���� ���� ������ �迭
	* @return ���� ����, ������ ���� Map
	*/
	TMap<EDirection, ECoverInfo> MakeCoverMap(TArray<FVector> CoverDirectionArr);

	/**
	* Score�� ���� ������ Action�� ��ȯ�մϴ�.
	* @param ActionScore - �׼� ���� ��
	* @return Action
	*/
	EAction GetActionAccordingToScore(int32 ActionScore) const;

	/**
	* Score�� ���� ������ Action�� ��ȯ�մϴ�.
	* @param ActionScore - �׼� ���� ��
	* @return Action
	*/
	void FindBestScoredAction(const TMap<ATile*, FAICommandInfo>& TileScoreBoard);

	void DebugAimingInfo(const FVector TileLocation, const int32 Score);

	/**
	* ��ǥ�� �ϴ� ��ġ�� ������ �������� ���θ� Ȯ���մϴ�. ( FOW �� ���� )
	* @param TargetLocation - ��ǥ�� �ϴ� ��ġ
	* @return ���� ���� ���� 
	*/
	bool CheckTargetLocation(FVector TargetLocation);

	/**
	* ��ġ�� ���� �� ���� ����� �����մϴ�.
	* @param TargetLocation - ��ǥ�� �ϴ� ��ġ
	*/
	void DecideWayToProceedBasedLocation(FVector TargetLocation);

	/**
	* �����ϴ� ������ �׾����� ȣ��˴ϴ�.
	*/
	UFUNCTION()
	void ControlledUnitDead(ACustomThirdPerson* ControlledUnit);

};