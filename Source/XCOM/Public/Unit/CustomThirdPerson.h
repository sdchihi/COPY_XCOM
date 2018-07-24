// Fill out your copy
// notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DestructibleWall.h"
#include "AimingComponent.h"
#include "FloatingWidget.h"
#include "CustomThirdPerson.generated.h"

class AGun;
class USpringArmComponent;
class UCameraComponent;
class UAimingComponent;
class UTrajectoryComponent;
class UHUDComponent;
enum class EDirection : uint8;

DECLARE_DYNAMIC_DELEGATE(FChangePlayerPawnDelegate);
DECLARE_DYNAMIC_DELEGATE(FStartActionDelegate);
DECLARE_DYNAMIC_DELEGATE_OneParam(FDeadCamDelegate, AActor*, TargetActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAfterActionDelegate, bool, bIsPlayerTeam);
DECLARE_DYNAMIC_DELEGATE_OneParam(FAfterMovingDelegate, ACustomThirdPerson*, MovingCharacter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUnprotectedMovingDelegate, FVector, Location);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FChangeViewTargetDelegateFromChar,const FVector, StartLoc, const FVector, TargetLoc);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FReadyToAttackDelegate, AActor*, TargetActor, FVector, AimDirection);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FStartShootingDelegate, AActor*, ShootingCharacter, bool, bPlayBlend);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FAnnounceDamageDelegate, AActor*, DamagedActor, float, Damage, FloatingWidgetState, State);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUnitDeadDelegate, ACustomThirdPerson*, DeadUnit);

UENUM(BlueprintType)
enum class EUnitState : uint8
{
	Idle,
	ReadyToAttack,
	Attack,
	GetHit,
	Dodge,
	Dead
};



UENUM(BlueprintType)
enum class EDirection : uint8
{
	South,
	North,
	East,
	West,
	None
};

UENUM(BlueprintType)
enum class EAction : uint8
{
	Attack,
	Vigilance,
	Ambush,
	Grenade,
	None
};

UENUM(BlueprintType)
enum class EWalkingState : uint8
{
	Running,
	Walk 
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

	FVector GetHeadLocation();

	EDirection CoverDirection = EDirection::None;
	
	UPROPERTY(BlueprintReadOnly)
	EDirection MovingDirectionDuringCover = EDirection::None;

	TMap<EDirection, ECoverInfo> CoverDirectionMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AGun> GunBlueprint;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<class AGrenade> GrenadeBlueprint;

	//void CheckAttackPotential(APawn* TargetPawn);

	bool GetTeamFlag() { return bTeam; };

	void ClearCoverDirectionInfo();

	UPROPERTY(BlueprintReadOnly)
	bool bIsCovering = false;

	UPROPERTY(BlueprintReadOnly)
	bool bInVisilance = false;

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

	FAfterActionDelegate AfterActionDelegate;

	FDeadCamDelegate DeadCamDelegate;

	FUnprotectedMovingDelegate UnprotectedMovingDelegate;

	FStartActionDelegate StartActionDelegate;

	FChangeViewTargetDelegateFromChar ChangeViewTargetDelegate;

	FAfterMovingDelegate AfterMovingDelegate;

	FReadyToAttackDelegate ReadyToAttackDelegate;

	FStartShootingDelegate StartShootingDelegate;

	FAnnounceDamageDelegate AnnounceDamageDelegate;

	FUnitDeadDelegate UnitDeadDelegate;

	void InformVisilanceSuccess(const FVector StartLocation, const FVector TargetLocation);

	UPROPERTY(EditDefaultsOnly)
	int32 RemainingActionPoint = 2;

	UFUNCTION(BlueprintImplementableEvent)
	void AimAt(FVector AimDirection, FName NewCollisionPresetName);

	UFUNCTION(BlueprintCallable)
	void RotateCharacter(float NewYaw, float LerpAlpha);

	UFUNCTION(BlueprintCallable)
	float DecideDirectionOfRotation(FVector AimDirection);


	UFUNCTION(BlueprintCallable)
	void StartFiring(FName NewCollisionPresetName);

	int32 GetStep() { return Step; };

	void UseActionPoint(int32 PointToUse);

	void RestoreActionPoint();

	void ScanEnemy();

	TArray<FAimingInfo>& GetAimingInfo() { return AimingInfo; };

	FAimingInfo GetSelectedAimingInfo() { return SelectedAimingInfo; };


	UPROPERTY(BlueprintReadOnly)
	FAimingInfo SelectedAimingInfo;

	UFUNCTION(BlueprintPure, BlueprintCallable)
	FVector GetShootingPlace() { return SelectedAimingInfo.StartLocation; };

	TMap<EAction, bool> GetPossibleAction() { return PossibleActionMap; };

	void AttackEnemyAccoringToIndex(const int32 TargetEnemyIndex);


	void AttackEnemyAccrodingToState(const FAimingInfo TargetAimingInfo);

	void StartTrajectory();

	void FinishTrajectory();

	void SetHealthBarVisibility(const bool bVisible);


	void InVigilance(const FVector TargetLocation);

	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable)
	void CoverMoving(FVector TargetLocation);

	UFUNCTION(BlueprintCallable)
	void AttackAfterCoverMoving();

	// AI를 위한 메소드
	UAimingComponent* GetAimingComponent() { return AimingComponent; };

	UFUNCTION(BlueprintCallable)
	void AfterShooting();

	void SetOffAttackState(const bool bExecuteDelegate);

	void StopVisilance();

	void BindVigilanceEvent(const TArray<ACustomThirdPerson*> OppositeTeamMember);

	UFUNCTION()
	void WatchOut(const FVector TargetLocation);


	//이동 메소드
	UFUNCTION()
	void MovingStepByStep(const int32 Progress);

	void RotateToNextTile(const FVector NextTileLocation);

	UFUNCTION()
	void MoveToNextTarget(const float LerpAlpha);

	UFUNCTION()
	void ArriveNextTile();

	void MoveToTargetTile(TArray<FVector>* OnTheWay , const int32 ActionPointToUse);


	// 순수  AI Code - > 차후 분리 필요
	
	void SetSpeed(float Speed) { this->Speed = Speed; };

	UFUNCTION(BlueprintCallable)
	float GetSpeed() { return Speed; }

	void ExecuteChangePawnDelegate();

	void SetWalkingState(EWalkingState WalkingStateToSet);

	bool IsInUnFoggedArea() const;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Montage")
	class UAnimMontage* LeftTurnMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Montage")
	UAnimMontage* RightTurnMontage;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Montage")
	UAnimMontage* TestDeadMontage;

	UFUNCTION(BlueprintCallable)
	void FinishDodge();

	AGun& GetGunReference() { return *GunReference; };

	void PrepareThrowGrenade(FVector Velocity);

	EUnitState GetUnitState() { return UnitState; };

	UPROPERTY(BlueprintReadOnly)
	EUnitState UnitState = EUnitState::Idle;

protected:
	
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser) override;

	EWalkingState WalkingState = EWalkingState::Running;

	virtual void FinishMoving();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UFogOfWarComponent* FOWComponent = nullptr;

	UPROPERTY()
	class UUnitHUD* HPBar = nullptr;

	UPROPERTY()
	UHUDComponent* HUDComponent = nullptr;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	AGun* GunReference;

private:
	UPROPERTY()
	UAimingComponent* AimingComponent = nullptr;

	
	UPROPERTY(EditDefaultsOnly)
	UTrajectoryComponent* TrajectoryComponent = nullptr;

	UPROPERTY()
	USkeletalMeshComponent* SkeletalMesh = nullptr;

	UPROPERTY(EditDefaultsOnly)
	bool bTeam = true;

	UPROPERTY(EditDefaultsOnly)
	int32 Step = 10;

	void InitializeTimeline();

	//
	UPROPERTY()
	class UTimelineComponent* MovingTimeline;

	UPROPERTY()
	UCurveFloat* FloatCurve;


	FVector RelativeCoverLoc = FVector(0,0,0);

	TArray<FAimingInfo> AimingInfo;

	//이후에 클래스로 묶일 능력치들
	int32 Aiming = 0.8;

	TMap<EAction, bool> PossibleActionMap;

	bool CheckTargetEnemyCoverState(const TMap<EAimingFactor, float>& TargetEnemyInfo);

	void DecideShootingChance(float& CriticalChance, float& DodgeChance);

	void StartSlowMotion();

	UFUNCTION()
	void FinishSlowMotion();

	void StartVisiliance();

	void SetMovingDirectionDuringCover(const FVector TargetLocation);

	void CoverUpAndAttack(const FAimingInfo TargetAimingInfo);

	UFUNCTION(BlueprintCallable)
	void Shoot();

	void UnderGuard();

	FVector PrevLocation;

	FVector NextLocation;

	// 순수  AI Code - > 차후 분리 필요

	TArray<FVector> PathToTargetTile;

	int32 ActionPointForMoving;

	int32 MovementIndex = 0;

	float Speed = 0;

	void ChangeTimelineFactor();

	void Dead();

	void ThrowGrenade(FVector Velocity, AGrenade* Grenade);

	FVector GetWrongDirection();

};
