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
	* ���� ������ ������ ������ ���ɴϴ�.
	* @param ActorLocation - ���� ����� �����ϴ� Actor�� ��ġ
	* @param AttackRadius - ���� ������ ����
	* @param bIsCover - �����ϴ� Actor�� ���� ���� ����
	* @param CoverDirectionMap - ���� ���� ������ �����ִ� ��
	* @param AimingInfoList - ���� ������ ���� ���� ������ ��ȯ���� Array
	*/
	void GetAttackableEnemyInfo(const FVector ActorLocation, const float AttackRadius, const bool bIsCover,const TMap<EDirection, ECoverInfo>& CoverDirectionMap, OUT TArray<FAimingInfo>& AimingInfoList);

	/**
	* ��� �� ���ذ����� ������ ������ ���ɴϴ�.
	* @param AttackRadius - ���� ������ ����
	* @param bIsCover - ���� Actor�� ���� ����
	* @param TargetLocation - ���� ����� ��ġ
	* @param CoverDirectionMap - �����ϰ� �ִ� ���󹰿� ���� Map
	* @param AimingInfo - ������ AimingInfo
	* @return ��� ���� ����
	*/
	bool GetVigilanceAimingInfo(const float AttackRadius, const bool bIsCover, const TMap<EDirection, ECoverInfo>& CoverDirectionMap, const FVector TargetLocation, OUT FAimingInfo& AimingInfo);

	/**
	* ������ ���������� ��ȯ�մϴ�..
	* @param ActorLocation - ���� Actor�� ��ġ
	* @param AttackRadius - ���� ������ ����
	* @param bIsCover - ���� Actor�� ���� ����
	* @param CoverDirectionMap - �����ϰ� �ִ� ���󹰿� ���� Map
	* @return AimingInfo
	*/
	FAimingInfo GetBestAimingInfo(const FVector ActorLocation, const float AttackRadius, const bool bIsCover, const TMap<EDirection, ECoverInfo>& CoverDirectionMap);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	* ���� ���� Ȯ���� ����մϴ�.
	* @param ActorLocation - ���� ����� �����ϴ� Actor�� ��ġ
	* @param HitResult - Linetrace ���
	* @param AttackRadius - ���� ������ ����
	* @param bIsCover - �����ϴ� Actor�� ���� ���� ����
	* @param AimingFactor - ���ؿ� �����ϴ� �ܺ� ��ҿ� ���� �����͸� ��ȯ�մϴ�
	* @param CriticalFactor - ũ��Ƽ�ÿ� �����ϴ� �ܺ� ��ҿ� ���� �����͸� ��ȯ�մϴ�.
	* @return ���� ���� Ȯ���� ��ȯ�մϴ�.
	*/
	float CalculateAttackSuccessRatio(const FVector ActorLocation, const FHitResult HitResult, float AttackRadius, const bool bIsCover, APawn* TargetPawn, TMap<EAimingFactor, float>& AimingFactor, TMap<ECriticalFactor, int8>& CriticalFactor);

	/**
	* ���� ����� �����ϰ��ִ� ���� ���ؼ��� �̷�� ������ ����մϴ�.
	* @param AimDirection - ���ؼ��� ���� �����Դϴ�.
	* @param TargetPawn - ���� ����� �Ǵ� Pawn�Դϴ�.
	* @param CoverInfo - ���� ����
	* @return ���� ���ؼ��� �̷�� ������ Degree�� ��ȯ�մϴ�.
	*/
	float CalculateAngleBtwAimAndWall(const FVector AimDirection, ACustomThirdPerson* TargetPawn, OUT ECoverInfo& CoverInfo);

	/**
	* ���� ���� �ȿ� �ִ� ���� ���ɴϴ�.
	* @param ActorLocation - ������ �����ϴ� Actor�� ��ġ
	* @param AttackRadius - ���� ���� ����
	* @param CharacterInRange - ���� ���� �ִ� ���ֵ��� �迭
	* @return ���� �ȿ� ���� �ִ��� ���θ� ��ȯ�մϴ�.
	*/
	bool GetEnemyInRange(const FVector ActorLocation, const float AttackRadius, OUT TArray<ACustomThirdPerson*>& CharacterInRange);

	/**
	* ���� ��Ȳ�� �´� LineTrace�� ���� ������ ������ ���͵��� ��ȯ�մϴ�
	* @param ActorLocation - ���� Actor�� ��ġ�Դϴ�.
	* @param CoverDirectionMap - ���� Actor�� �����ϰ� �ִ� ���󹰿� ���� ������
	* @param EnemiesInRange - ���� ���� �ȿ� �����ִ� ���� Array
	* @param bIsCovering - ���� Actor�� ���� ����
	* @param SensibleEnemyInfo - LineTrace�� ���� Ȯ�ε� ���� Array
	* @return ������ �������� ���θ� ��ȯ�մϴ�.
	*/
	bool FilterAttackableEnemy(const FVector ActorLocation, const TMap<EDirection, ECoverInfo>& CoverDirectionMap, const TArray<ACustomThirdPerson*>& EnemiesInRange, const bool bIsCovering, OUT TArray<FHitResult>& SensibleEnemyInfo);

	/**
	* ���ؿ� �ʿ��� LineTrace�� �����մϴ�.
	* @param StartLocation - LineTrace �� ������
	* @param TargetLocation - LineTrace �� ����� �Ǵ� Ÿ���� ��ġ
	* @return LineTrace ����� ��ȯ�մϴ�.
	*/
	FHitResult LineTraceWhenAiming(const FVector StartLocation, const FVector TargetLocation);

	/**
	* ������ ���� ���� ��ġ������ LineTrace����� ���ɴϴ�.
	* @param ActorLocation - �����ϴ� Actor�� ��ġ
	* @param SurroundingArea - ���� ��Ȳ���� ������ ���� ���� �ĺ� ��ġ
	* @param AimingInfo - LineTrace�� ����� ��ȯ���� Array
	*/
	void GetAimingInfoFromSurroundingArea(const FVector ActorLocation, const FVector SurroundingArea, TArray<FHitResult>&  AimingInfo);
	
	/**
	* ���� ������ Rotator������ ���ɴϴ�.
	* @param DirectionAndInfoPair - Map ���� ���� Direction, CoverInfo Pair
	* @return ���� ������ Rotator��
	*/
	FRotator FindCoverDirection(TPair<EDirection, ECoverInfo> DirectionAndInfoPair);

	/**
	* ���� �����͸� ���͸��Ͽ� ������ ���� �����͵��� ��ȯ�մϴ�.
	* @param AllCaseInfo - ���͸����� ���� Aiming Info Array
	* @param BestCaseArr - ���͸� �� ��ȯ��  AimingInfo array
	* @param TargetLocArr - Ÿ���� ��ġ Array
	*/
	void FindBestCaseInAimingInfo(const TArray<FAimingInfo> AllCaseInfo, TArray<FAimingInfo>& BestCase, const TArray<FVector> TargetLocArr);
	
	/**
	* ���� Actor�� ���ؿ� ����� TraceChannel�� �����ɴϴ�.
	* @return ECollisionChannel ���� ä��
	*/
	ECollisionChannel GetAimingChannel();
};
