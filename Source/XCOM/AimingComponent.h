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

USTRUCT()
struct FAimingInfo
{
	GENERATED_BODY()

	FVector StartLocation;

	FVector TargetLocation;

	float Probability;

	TMap<EAimingFactor, float> Factor;

	FAimingInfo()
	{
		StartLocation = FVector(0, 0, 0);	
		TargetLocation = FVector(0, 0, 0);
		Probability = 0;
	}

	FAimingInfo(FVector StartLoc, FVector TargetLoc, float SucessProbability, TMap<EAimingFactor, float>& AimingFactor)
	{
		StartLocation = StartLoc;
		TargetLocation = TargetLoc;
		Probability = SucessProbability;
		Factor = AimingFactor;
	}

	void Clear() 
	{
		StartLocation = FVector(0, 0, 0);
		TargetLocation = FVector(0, 0, 0);
		Probability = 0;
		Factor.Empty();
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
	// Sets default values for this component's properties
	UAimingComponent();

	void GetAttackableEnemyInfo(const float AttackRadius, const bool bIsCover,const TMap<EDirection, ECoverInfo>& CoverDirectionMap, OUT TArray<FAimingInfo>& AimingInfoList);

	bool GetVigilanceAimingInfo(const float AttackRadius, const bool bIsCover, const TMap<EDirection, ECoverInfo>& CoverDirectionMap, const FVector TargetLocation, OUT FAimingInfo& AimingInfo);

	FAimingInfo GetBestAimingInfo(const float AttackRadius, const bool bIsCover, const TMap<EDirection, ECoverInfo>& CoverDirectionMap);


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	float CalculateAttackSuccessRatio(const FHitResult HitResult, float AttackRadius, APawn* TargetPawn, TMap<EAimingFactor, float>& AimingFactor);

	float CalculateAngleBtwAimAndWall(const FVector AimDirection, ACustomThirdPerson* TargetPawn, OUT ECoverInfo& CoverInfo);

	bool GetEnemyInRange(const float AttackRadius, OUT TArray<ACustomThirdPerson*>& CharacterInRange);

	bool FilterAttackableEnemy(const TMap<EDirection, ECoverInfo>& CoverDirectionMap, const TArray<ACustomThirdPerson*>& EnemiesInRange, const bool bIsCovering, OUT TArray<FHitResult>& SensibleEnemyInfo);

	FHitResult LineTraceWhenAiming(const FVector StartLocation, const FVector TargetLocation);

	void GetAimingInfoFromSurroundingArea(const FVector SurroundingArea, TArray<FHitResult>&  AimingInfo);

	FRotator FindCoverDirection(TPair<EDirection, ECoverInfo> DirectionAndInfoPair);

	TArray<FAimingInfo> AimingInfoList;

	void FindBestCaseInAimingInfo(const TArray<FAimingInfo> AllCaseInfo, TArray<FAimingInfo>& BestCase, const TArray<FVector> TargetLocArr);
	
};
