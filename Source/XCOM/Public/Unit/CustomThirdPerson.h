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

	/**
	* ���� �ð� ��� �� �׼� ����Ʈ�� ����մϴ�.
	* @param Time - ���� �ð�
	* @param Point - ����� �׼� ����Ʈ ��
	*/
	void UseActionPointAfterDelay(float Time, int32 Point);

	/** Dead Animation ���� �� �ʿ��ϸ� ���� �ѱ�ϴ�.*/
	UFUNCTION(BlueprintCallable)
	void AfterDeadAnimation();
public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** ���� ���� ���� */
	EDirection CoverDirection = EDirection::None;
	
	/** ���� ���� �� �̵� ���� */
	UPROPERTY(BlueprintReadOnly)
	EDirection MovingDirectionDuringCover = EDirection::None;

	/** ���� ���� ������ ��� ���� / ���� ������ ���� �� */
	TMap<EDirection, ECoverInfo> CoverDirectionMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AGun> GunBlueprint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<class AGrenade> GrenadeBlueprint;

	bool GetTeamFlag() { return bTeam; };

	/** ���� map �� �ʱ�ȭ�մϴ�. */
	void ClearCoverDirectionInfo();

	/** ���� ���������� ���� */
	UPROPERTY(BlueprintReadOnly)
	bool bIsCovering = false;

	/** ���� ��������� ���� */
	UPROPERTY(BlueprintReadOnly)
	bool bInVisilance = false;	
	
	/** ���� ������ Ȱ���������� ���� */
	bool bCanAction = true;

	/** �ִ� HP */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int MaxHP = 5;

	/** ���� HP */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int CurrentHP;

	bool IsDead();

	/** ���� ������ ��Ÿ� */
	UPROPERTY(EditDefaultsOnly)
	float AttackRadius = 1000;

	/** ���� �̵� ������ ���� �� */
	int32 CurrentMovableStep;
	
	/** �׼� ����Ʈ �ϳ��� �̵� ������ ���� �� */
	int32 GetMovableStepPerActionPoint() { return Step / 2; };
	
	/** ����� �ùٸ� �ִϸ��̼��� ���� ���� ���� ȸ���մϴ�. */
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

	/** ���� �׼� ����Ʈ */
	UPROPERTY(EditDefaultsOnly)
	int32 RemainingActionPoint = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Montage")
	class UAnimMontage* LeftTurnMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Montage")
	UAnimMontage* RightTurnMontage;

	/**
	* ��� �� ���� �߰������� ī�޶� �����Ű�� Delegate �Լ��� ȣ���մϴ�.
	* @param StartLocation - ī�޶� ��ġ
	* @param TargetLocation - ��ǥ ����
	*/
	void InformVisilanceSuccess(const FVector StartLocation, const FVector TargetLocation);

	UFUNCTION(BlueprintImplementableEvent)
	void AimAt(FVector AimDirection, FName NewCollisionPresetName);

	/**
	* �������Ʈ���� ȣ��� �� �ִ� �޼ҵ�� ���� �ð����� ĳ���͸� ��ǥ �������� ȸ����ŵ�ϴ�. (Timeline�� �����ؼ� ���)
	* @param NewYaw - ��ǥ�� �ϴ� Yaw���Դϴ�.
	* @param LerpAlpha - Lerp�� ���� Alpha���Դϴ�.
	*/
	UFUNCTION(BlueprintCallable)
	void RotateCharacter(float NewYaw, float LerpAlpha);

	/**
	* �����ϴ� ���⿡ ���� ĳ������ ȸ�� ������ �����մϴ�.
	* @param AimDirection - ���� ���� �����Դϴ�.
	* @return NewYaw - ��ǥ�� �ϴ� Yaw���Դϴ�.
	*/
	UFUNCTION(BlueprintCallable)
	float DecideDirectionOfRotation(FVector AimDirection);

	/**
	* ������ �����մϴ�.
	* @param NewCollisionPresetName - ����ü�� ����� �浹 ������ �̸�
	*/
	UFUNCTION(BlueprintCallable)
	void StartFiring(FName NewCollisionPresetName);

	int32 GetStep() { return Step; };

	/**
	* �׼� ����Ʈ�� ����մϴ�.
	* @param PointToUse - ����� �׼� ����Ʈ ��
	*/
	UFUNCTION()
	void UseActionPoint(int32 PointToUse);

	/** �׼� ����Ʈ�� ȸ���Ѵ�. */
	void RestoreActionPoint();

	/** �ֺ��� ��ĵ�� ���� ������ ���鿡 ���� ������ �����մϴ�. */
	void ScanEnemy();

	TArray<FAimingInfo>& GetAimingInfo() { return AimingInfo; };

	FAimingInfo GetSelectedAimingInfo() { return SelectedAimingInfo; };

	/** ������ ����Ҷ� ������ Aiming Info */
	UPROPERTY(BlueprintReadOnly)
	FAimingInfo SelectedAimingInfo;

	UFUNCTION(BlueprintPure, BlueprintCallable)
	FVector GetShootingPlace() { return SelectedAimingInfo.StartLocation; };

	TMap<EAction, bool> GetPossibleAction() { return PossibleActionMap; };

	/**
	* Widget���κ��� �޾ƿ� Index�� ���� ����, �����մϴ�.
	* @param TargetEnemyIndex - AimingInfo Index
	*/
	void AttackEnemyAccoringToIndex(const int32 TargetEnemyIndex);

	/**
	* �Ķ���ͷ� �޾ƿ� AimingInfo�� �����մϴ�.
	* @param TargetAimingInfo
	*/
	void AttackEnemyAccrodingToState(const FAimingInfo TargetAimingInfo);

	void StartTrajectory();

	void FinishTrajectory();

	/**
	* HPbar �� ���ü��� �����մϴ�.
	* @param bVisible - ���ӿ� HPBar�� ������ �Ⱥ����� ����
	*/
	void SetHealthBarVisibility(const bool bVisible);

	/**
	* ������϶� ���� �����ӿ� �����մϴ�.
	* @param TargetLocation - ��ǥ ����
	*/
	void InVigilance(const FVector TargetLocation);

	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable)
	void CoverMoving(FVector TargetLocation);

	/** Cover ���¿��� �������� ���� �� ������ �����մϴ�.*/
	UFUNCTION(BlueprintCallable)
	void AttackAfterCoverMoving();

	// AI�� ���� �޼ҵ�
	UAimingComponent* GetAimingComponent() { return AimingComponent; };

	/** ��� �Ϸ� �� �ʿ��� �۾��� �����մϴ�. */
	UFUNCTION(BlueprintCallable)
	void AfterShooting();

	/**
	* ������ ���� �� ��������Ʈ ����, Flag ��ȯ
	* @param bExecuteDelegate - Delegate ���� ����
	*/
	void SetOffAttackState(const bool bExecuteDelegate);

	/** ��� Ȱ���� ����ϴ�.*/
	void StopVisilance();

	/**
	* ���� �̵��� �����ϵ��� Delegate ���ε��� �մϴ�.
	* @param OppositeTeamMember - ī�޶� ��ġ
	*/
	void BindVigilanceEvent(const TArray<ACustomThirdPerson*> OppositeTeamMember);

	UFUNCTION()
	void WatchOut(const FVector TargetLocation);

	/**
	* ��Ÿ�Ͼ� �̵��մϴ�.
	* @param Progress - ��ǥ Ÿ�ϱ����� ���൵
	*/
	UFUNCTION()
	void MovingStepByStep(const int32 Progress);

	/**
	* ���� Ÿ�� �������� ĳ���͸� ȸ����ŵ�ϴ�..
	* @param NextTileLocation - ���� Ÿ���� ��ġ
	*/
	void RotateToNextTile(const FVector NextTileLocation);

	/**
	* ���� ��ǥ�������� �̵���ŵ�ϴ�. (Lerp)
	* @param LerpAlpha
	*/
	UFUNCTION()
	void MoveToNextTarget(const float LerpAlpha);

	/** ��ǥ�� �ϴ� Ÿ�Ͽ� ���������� ȣ��˴ϴ�. */
	UFUNCTION()
	void ArriveNextTile();

	/**
	* ��ǥ Ÿ�Ϸ� �̵��մϴ�.
	* @param OnTheWay - ��θ� ���� �迭
	* @param ActionPointToUse - ����� �׼� ����Ʈ
	*/
	void MoveToTargetTile(TArray<FVector>* OnTheWay , const int32 ActionPointToUse);
	
	void SetSpeed(float Speed) { this->Speed = Speed; };

	UFUNCTION(BlueprintCallable)
	float GetSpeed() { return Speed; }

	/** �ٸ� ���ֿ��� Ȱ��ȭ�� �ѱ�ϴ�.*/
	void ExecuteChangePawnDelegate();

	void SetWalkingState(EWalkingState WalkingStateToSet);

	/**
	* ������ �Ȱ��ӿ� �ִ��� ���θ� Ȯ���մϴ�.
	* @return �Ȱ��ӿ� �ִ��� ����
	*/
	bool IsInUnFoggedArea() const;
	
	UFUNCTION(BlueprintCallable)
	void FinishDodge();

	AGun& GetGunReference() { return *GunReference; };

	/**
	* ����ź ��ô�� �غ��մϴ�.
	* @param Velocity - ����ź�� ���� ���� ����
	*/
	void PrepareThrowGrenade(FVector Velocity);

	EUnitState GetUnitState() { return UnitState; };

	UPROPERTY(BlueprintReadOnly)
	EUnitState UnitState = EUnitState::Idle;

	/** ��� ������ �˸��ϴ�.*/
	void AnnounceVisilance();
	 
	UPROPERTY()
	UHUDComponent* HUDComponent = nullptr;

	/**
	* ���� �޽��� �Ӹ� ��ġ�� ���ɴϴ�.
	* @return ���� �޽��� �Ӹ� ���� ��ǥ
	*/
	FVector GetHeadLocation();

protected:
	/** ���� �̵��ϴ� ����� ������ State*/
	EWalkingState WalkingState = EWalkingState::Running;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UFogOfWarComponent* FOWComponent = nullptr;

	UPROPERTY()
	class UUnitHUD* HPBar = nullptr;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	AGun* GunReference;

	/** ������ ��������� ȣ��˴ϴ�.*/
	virtual void Dead();

	/** �̵��� �����մϴ�.*/
	virtual void FinishMoving();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser) override;
private:
	UPROPERTY()
	UAimingComponent* AimingComponent = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	UTrajectoryComponent* TrajectoryComponent = nullptr;

	UPROPERTY()
	USkeletalMeshComponent* SkeletalMesh = nullptr;

	/** ������ ���� �����ϴ� �÷��� ( �÷��̾��� / AI �� ) */
	UPROPERTY(EditDefaultsOnly)
	bool bTeam = true;

	/** �̵� ������ ���� ��  */
	UPROPERTY(EditDefaultsOnly)
	int32 Step = 10;


	UPROPERTY()
	class UTimelineComponent* MovingTimeline;

	UPROPERTY()
	UCurveFloat* FloatCurve;

	FVector RelativeCoverLoc = FVector(0,0,0);

	/** �ĺ��� �����ִ� ��� ���� ������ �迭�Դϴ� */
	TArray<FAimingInfo> AimingInfo;

	//���Ŀ� Ŭ������ ���� �ɷ�ġ��
	int32 Aiming = 0.8;

	/** ������ �� �� �ִ� �׼��� Map�Դϴ�. */
	TMap<EAction, bool> PossibleActionMap;

	/** ���� ��ġ */
	FVector PrevLocation;

	/** ���� ��ġ */
	FVector NextLocation;

	/** ��θ� ã�Ƽ� �̵��ϴµ� �ʿ��� �迭 */
	TArray<FVector> PathToTargetTile;

	/** �̵��ϴµ� ����� �׼� ����Ʈ */
	int32 ActionPointForMoving;

	int32 MovementIndex = 0;

	/** �ִϸ��̼ǿ� ����� �ӵ� �� */
	float Speed = 0;

	/** �̵��� �ʿ��� Timeline�� �ʱ�ȭ�մϴ�. */
	void InitializeTimeline();

	/**
	* �������� ���� �ִ��� Ȯ��
	* @param TargetEnemyInfo - EAimingFactor / float
	* @return �������� ���� �ִ��� ����
	*/
	bool CheckTargetEnemyCoverState(const TMap<EAimingFactor, float>& TargetEnemyInfo);

	/**
	* ġ��Ÿ��, ȸ������ �����մϴ�.
	* @param CriticalChance - ġ��Ÿ��
	* @param DodgeChance - ȸ����
	*/
	void DecideShootingChance(OUT float& CriticalChance, OUT float& DodgeChance);

	/** ������ ������ ������ TimeDilation�� ���Դϴ� (���� ���� ���忡 ���ο� ��� ����) */
	void StartSlowMotion();

	/** ������ TimeDilation�� �����·� �����մϴ� (GlobalTimeDilation = 1) */
	UFUNCTION()
	void FinishSlowMotion();

	/**
	* ������ �̵��Ҷ� ������ �����մϴ�.InformVisilanceSuccess
	* @param TargetLocation - ��ǥ ����
	*/
	void SetMovingDirectionDuringCover(const FVector TargetLocation);

	/**
	* ���� �̵� �� �����մϴ�.
	* @param TargetAimingInfo - AimingInfo
	*/
	void CoverUpAndAttack(const FAimingInfo TargetAimingInfo);

	/** ������ ����մϴ�.*/
	UFUNCTION(BlueprintCallable)
	void Shoot();

	/** ���� State�� ���� �̵��� ���õ� Timeline�� �����մϴ�. */
	void ChangeTimelineFactor();

	/**
	* ����ź�� ��ô�մϴ�.
	* @param Velocity - ����ź�� ���� ���� ����
	* @param Grenade - ���� ����ź Actor pointer
	*/
	void ThrowGrenade(FVector Velocity, AGrenade* Grenade);

	/**
	* ��ǥ���� ������ ������ ���Ҷ� ȣ���մϴ�.
	* @return ��ݽ� �߸��� ����
	*/
	FVector GetWrongDirection();

	/** Mesh�� ������ ������Ʈ���� �����մϴ�. */
	void DestroyUnnecessaryComponents();

};
