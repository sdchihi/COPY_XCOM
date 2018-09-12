// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AimingComponent.generated.h"

UENUM(BlueprintType)
enum class EAimingFactor : uint8
{
	AimingAbility,
	GoodAngle,
	FullCover,
	HalfCover,
	Disatnce
};

UENUM(BlueprintType)
enum class ECriticalFactor : uint8
{
	WeaponAbility,
	SideAttack
};



USTRUCT()
struct FAimingInfo
{
	GENERATED_BODY()

	FVector StartLocation;

	FVector TargetLocation;

	float Probability;

	AActor* TargetActor;

	TMap<EAimingFactor, float> Factor;

	TMap<ECriticalFactor, int8> CriticalFactor;

	FAimingInfo()
	{
		StartLocation = FVector(0, 0, 0);	
		TargetLocation = FVector(0, 0, 0);
		Probability = 0;
	}

	FAimingInfo(FVector StartLoc, FVector TargetLoc, float SucessProbability, AActor* Target)
	{
		StartLocation = StartLoc;
		TargetLocation = TargetLoc;
		Probability = SucessProbability;
		TargetActor = Target;
	}

	FAimingInfo(FVector StartLoc, FVector TargetLoc, float SucessProbability, AActor* Target,TMap<EAimingFactor, float>& AimingFactor )
	{
		StartLocation = StartLoc;
		TargetLocation = TargetLoc;
		Probability = SucessProbability;
		Factor = AimingFactor;
		TargetActor = Target;
	}

	FAimingInfo(FVector StartLoc, FVector TargetLoc, float SucessProbability, AActor* Target, TMap<EAimingFactor, float>& AimingFactor, TMap<ECriticalFactor, int8>& CriticalFactorMap)
	{
		StartLocation = StartLoc;
		TargetLocation = TargetLoc;
		Probability = SucessProbability;
		Factor = AimingFactor;
		TargetActor = Target;
		CriticalFactor = CriticalFactorMap;
	}

	void Clear() 
	{
		StartLocation = FVector(0, 0, 0);
		TargetLocation = FVector(0, 0, 0);
		Probability = 0;
		Factor.Empty();
		CriticalFactor.Empty();
		TargetActor = nullptr;
	}

	float SumOfCriticalProb() 
	{
		float Sum = 0;
		for (auto It = CriticalFactor.CreateConstIterator(); It; ++It)
		{
			Sum += It.Value();
		}
		return Sum / 100;
	}
};


enum class EDirection : uint8;
enum class ECoverInfo : uint8;

class ACustomThirdPerson;

UCLASS( ClassGroup=(XCOM), meta=(BlueprintSpawnableComponent) )
class XCOM_API UAimingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAimingComponent();

	/**
	* 조준 가능한 적들의 정보를 얻어옵니다.
	* @param ActorLocation - 조준 기능을 실행하는 Actor의 위치
	* @param AttackRadius - 조준 가능한 범위
	* @param bIsCover - 실행하는 Actor가 엄폐 상태 여부
	* @param CoverDirectionMap - 엄폐에 대한 정보를 갖고있는 맵
	* @param AimingInfoList - 조준 가능한 적에 대한 정보를 반환해줄 Array
	*/
	void GetAttackableEnemyInfo(const FVector ActorLocation, const float AttackRadius, const bool bIsCover,const TMap<EDirection, ECoverInfo>& CoverDirectionMap, OUT TArray<FAimingInfo>& AimingInfoList);

	/**
	* 경계 중 조준가능한 적들의 정보를 얻어옵니다.
	* @param AttackRadius - 조준 가능한 범위
	* @param bIsCover - 현재 Actor의 엄폐 여부
	* @param TargetLocation - 공격 대상의 위치
	* @param CoverDirectionMap - 엄폐하고 있는 엄폐물에 대한 Map
	* @param AimingInfo - 갱신할 AimingInfo
	* @return 사격 가능 여부
	*/
	bool GetVigilanceAimingInfo(const float AttackRadius, const bool bIsCover, const TMap<EDirection, ECoverInfo>& CoverDirectionMap, const FVector TargetLocation, OUT FAimingInfo& AimingInfo);

	/**
	* 최적의 조준정보를 반환합니다..
	* @param ActorLocation - 현재 Actor의 위치
	* @param AttackRadius - 공격 가능한 범위
	* @param bIsCover - 현재 Actor의 엄폐 여부
	* @param CoverDirectionMap - 엄폐하고 있는 엄폐물에 대한 Map
	* @return AimingInfo
	*/
	FAimingInfo GetBestAimingInfo(const FVector ActorLocation, const float AttackRadius, const bool bIsCover, const TMap<EDirection, ECoverInfo>& CoverDirectionMap);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	* 공격 성공 확률을 계산합니다.
	* @param ActorLocation - 조준 기능을 실행하는 Actor의 위치
	* @param HitResult - Linetrace 결과
	* @param AttackRadius - 조준 가능한 범위
	* @param bIsCover - 실행하는 Actor가 엄폐 상태 여부
	* @param AimingFactor - 조준에 관여하는 외부 요소에 대한 데이터를 반환합니다
	* @param CriticalFactor - 크리티컬에 관여하는 외부 요소에 대한 데이터를 반환합니다.
	* @return 공격 성공 확률을 반환합니다.
	*/
	float CalculateAttackSuccessRatio(const FVector ActorLocation, const FHitResult HitResult, float AttackRadius, const bool bIsCover, APawn* TargetPawn, TMap<EAimingFactor, float>& AimingFactor, TMap<ECriticalFactor, int8>& CriticalFactor);

	/**
	* 공격 대상이 엄폐하고있는 벽과 조준선이 이루는 각도를 계산합니다.
	* @param AimDirection - 조준선의 방향 벡터입니다.
	* @param TargetPawn - 공격 대상이 되는 Pawn입니다.
	* @param CoverInfo - 엄폐 정보
	* @return 벽과 조준선이 이루는 각도를 Degree로 반환합니다.
	*/
	float CalculateAngleBtwAimAndWall(const FVector AimDirection, ACustomThirdPerson* TargetPawn, OUT ECoverInfo& CoverInfo);

	/**
	* 공격 범위 안에 있는 적을 얻어옵니다.
	* @param ActorLocation - 조준을 실행하는 Actor의 위치
	* @param AttackRadius - 공격 가능 범위
	* @param CharacterInRange - 범위 내에 있는 유닛들의 배열
	* @return 범위 안에 적이 있는지 여부를 반환합니다.
	*/
	bool GetEnemyInRange(const FVector ActorLocation, const float AttackRadius, OUT TArray<ACustomThirdPerson*>& CharacterInRange);

	/**
	* 엄폐 상황에 맞는 LineTrace를 통해 조준이 가능한 액터들을 반환합니다
	* @param ActorLocation - 현재 Actor의 위치입니다.
	* @param CoverDirectionMap - 현재 Actor가 엄폐하고 있는 엄폐물에 대한 데이터
	* @param EnemiesInRange - 공격 범위 안에 들어와있는 적의 Array
	* @param bIsCovering - 현재 Actor의 엄폐 여부
	* @param SensibleEnemyInfo - LineTrace를 통해 확인된 적의 Array
	* @return 조준이 가능한지 여부를 반환합니다.
	*/
	bool FilterAttackableEnemy(const FVector ActorLocation, const TMap<EDirection, ECoverInfo>& CoverDirectionMap, const TArray<ACustomThirdPerson*>& EnemiesInRange, const bool bIsCovering, OUT TArray<FHitResult>& SensibleEnemyInfo);

	/**
	* 조준에 필요한 LineTrace를 수행합니다.
	* @param StartLocation - LineTrace 의 시작점
	* @param TargetLocation - LineTrace 의 대상이 되는 타겟의 위치
	* @return LineTrace 결과를 반환합니다.
	*/
	FHitResult LineTraceWhenAiming(const FVector StartLocation, const FVector TargetLocation);

	/**
	* 엄폐중 공격 가능 위치에서의 LineTrace결과를 얻어옵니다.
	* @param ActorLocation - 조준하는 Actor의 위치
	* @param SurroundingArea - 엄폐 상황에서 가능한 공격 시작 후보 위치
	* @param AimingInfo - LineTrace의 결과를 반환받을 Array
	*/
	void GetAimingInfoFromSurroundingArea(const FVector ActorLocation, const FVector SurroundingArea, TArray<FHitResult>&  AimingInfo);
	
	/**
	* 엄폐 방향을 Rotator값으로 얻어옵니다.
	* @param DirectionAndInfoPair - Map 에서 얻어온 Direction, CoverInfo Pair
	* @return 엄폐 방향의 Rotator값
	*/
	FRotator FindCoverDirection(TPair<EDirection, ECoverInfo> DirectionAndInfoPair);

	/**
	* 조준 데이터를 필터링하여 최적의 조준 데이터들을 반환합니다.
	* @param AllCaseInfo - 필터링되지 않은 Aiming Info Array
	* @param BestCaseArr - 필터링 후 반환할  AimingInfo array
	* @param TargetLocArr - 타겟의 위치 Array
	*/
	void FindBestCaseInAimingInfo(const TArray<FAimingInfo> AllCaseInfo, TArray<FAimingInfo>& BestCase, const TArray<FVector> TargetLocArr);
	
	/**
	* 현재 Actor가 조준에 사용할 TraceChannel을 가져옵니다.
	* @return ECollisionChannel 조준 채널
	*/
	ECollisionChannel GetAimingChannel();
};
