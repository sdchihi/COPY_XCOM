// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DestructibleWall.h"
#include "CustomThirdPerson.generated.h"

class AGun;
class USpringArmComponent;
class UCameraComponent;

DECLARE_DYNAMIC_DELEGATE(FChangePlayerPawnDelegate);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCheckTurnDelegate , bool , bIsPlayerTeam);


UENUM(BlueprintType)
enum class ECoverDirection : uint8
{
	South,
	North,
	East,
	West,
	None
};

USTRUCT()
struct FAimingInfo
{
	GENERATED_BODY()

	FVector StartLocation;

	FVector TargetLocation;

	float Probability;


	FAimingInfo()
	{
		StartLocation = FVector(0, 0, 0);
		TargetLocation = FVector(0, 0, 0);
		Probability = 0;
	}

	FAimingInfo(FVector StartLoc, FVector TargetLoc, float SucessProbability)
	{
		StartLocation = StartLoc;
		TargetLocation = TargetLoc;
		Probability = SucessProbability;
	}
};


UCLASS()
class XCOM_API ACustomThirdPerson : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACustomThirdPerson();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	ECoverDirection CoverDirection = ECoverDirection::None;
	
	TMap<ECoverDirection, ECoverInfo> CoverDirectionMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AGun> GunBlueprint;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	AGun* GunReference;

	void CheckAttackPotential(APawn* TargetPawn);

	bool GetTeamFlag() { return bTeam; };

	void ClearCoverDirectionInfo();

	UPROPERTY(BlueprintReadOnly)
	bool bIsCovering = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsAttack = false;

	bool bCanAction = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int MaxHP = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int CurrentHP;

	UPROPERTY(EditDefaultsOnly)
	float AttackRadius = 1000;

	int32 CurrentMovableStep;
	
	int32 GetMovableStepPerActionPoint() { return Step / 2; };

	void RotateTowardWall();

	FChangePlayerPawnDelegate ChangePlayerPawnDelegate;

	FCheckTurnDelegate CheckTurnDelegate;

	UPROPERTY(EditDefaultsOnly)
	int32 RemainingActionPoint = 2;

	UFUNCTION(BlueprintImplementableEvent)
	void AimAt(FVector AimDirection, FName NewCollisionPresetName);

	UFUNCTION(BlueprintCallable)
	void RotateCharacter(FVector AimDirection, float LerpAlpha);

	UFUNCTION(BlueprintCallable)
	void StartFiring(FName NewCollisionPresetName);

	int32 GetStep() { return Step; };

	void UseActionPoint(int32 PointToUse);

	void RestoreActionPoint();

	void GetAttackableEnemyInfo();

protected:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser) override;

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* SpringArm= nullptr;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	bool bTeam = true;

	UPROPERTY(EditDefaultsOnly)
	int32 Step = 10;

	UPROPERTY(EditDefaultsOnly)
	float AttackRange = 500;

	float CalculateAttackSuccessRatio(const FHitResult HitResult, APawn* TargetPawn);

	float CalculateAngleBtwAimAndWall(const FVector AimDirection, ACustomThirdPerson* TargetPawn, OUT ECoverInfo& CoverInfo);

	void SetOffAttackState();

	FVector RelativeCoverLoc;

	//이후에 클래스로 묶일 능력치들
	int32 Aiming = 80;

	bool GetEnemyInRange(OUT TArray<ACustomThirdPerson*>& CharacterInRange);

	bool FilterAttackableEnemy(const TArray<ACustomThirdPerson*>& EnemiesInRange, OUT TArray<FHitResult>& SensibleEnemyInfo);

	FHitResult LineTraceWhenAiming(const FVector StartLocation,const FVector TargetLocation);

	void GetAimingInfoFromSurroundingArea(const FVector SurroundingArea, TArray<FHitResult>&  AimingInfo);

	FRotator FindCoverDirection(TPair<ECoverDirection, ECoverInfo> DirectionAndInfoPair);

	TArray<FAimingInfo> AimingInfoList;

	void FindBestCaseInAimingInfo(const TArray<FAimingInfo> AllCaseInfo, TArray<FAimingInfo>& BestCase, const TArray<FVector> TargetLocArr);

};
