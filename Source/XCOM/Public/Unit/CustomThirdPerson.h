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
	* 일정 시간 경과 후 액션 포인트를 사용합니다.
	* @param Time - 지연 시간
	* @param Point - 사용할 액션 포인트 값
	*/
	void UseActionPointAfterDelay(float Time, int32 Point);

	/** Dead Animation 종료 후 필요하면 턴을 넘깁니다.*/
	UFUNCTION(BlueprintCallable)
	void AfterDeadAnimation();
public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** 현재 엄폐 방향 */
	EDirection CoverDirection = EDirection::None;
	
	/** 현재 엄폐 중 이동 방향 */
	UPROPERTY(BlueprintReadOnly)
	EDirection MovingDirectionDuringCover = EDirection::None;

	/** 현재 엄폐 가능한 모든 방향 / 엄폐 종류에 대한 맵 */
	TMap<EDirection, ECoverInfo> CoverDirectionMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AGun> GunBlueprint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<class AGrenade> GrenadeBlueprint;

	bool GetTeamFlag() { return bTeam; };

	/** 엄폐 map 을 초기화합니다. */
	void ClearCoverDirectionInfo();

	/** 현재 엄폐중인지 여부 */
	UPROPERTY(BlueprintReadOnly)
	bool bIsCovering = false;

	/** 현재 경계중인지 여부 */
	UPROPERTY(BlueprintReadOnly)
	bool bInVisilance = false;	
	
	/** 현재 유닛이 활동가능한지 여부 */
	bool bCanAction = true;

	/** 최대 HP */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int MaxHP = 5;

	/** 현재 HP */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int CurrentHP;

	bool IsDead();

	/** 공격 가능한 사거리 */
	UPROPERTY(EditDefaultsOnly)
	float AttackRadius = 1000;

	/** 현재 이동 가능한 걸음 수 */
	int32 CurrentMovableStep;
	
	/** 액션 포인트 하나로 이동 가능한 걸음 수 */
	int32 GetMovableStepPerActionPoint() { return Step / 2; };
	
	/** 엄폐시 올바른 애니메이션을 위해 벽을 향해 회전합니다. */
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

	/** 남은 액션 포인트 */
	UPROPERTY(EditDefaultsOnly)
	int32 RemainingActionPoint = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Montage")
	class UAnimMontage* LeftTurnMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Montage")
	UAnimMontage* RightTurnMontage;

	/**
	* 경계 중 적을 발견했을때 카메라를 변경시키는 Delegate 함수를 호출합니다.
	* @param StartLocation - 카메라 위치
	* @param TargetLocation - 목표 지점
	*/
	void InformVisilanceSuccess(const FVector StartLocation, const FVector TargetLocation);

	UFUNCTION(BlueprintImplementableEvent)
	void AimAt(FVector AimDirection, FName NewCollisionPresetName);

	/**
	* 블루프린트에서 호출될 수 있는 메소드로 일정 시간내에 캐릭터를 목표 방향으로 회전시킵니다. (Timeline과 연계해서 사용)
	* @param NewYaw - 목표로 하는 Yaw값입니다.
	* @param LerpAlpha - Lerp에 사용될 Alpha값입니다.
	*/
	UFUNCTION(BlueprintCallable)
	void RotateCharacter(float NewYaw, float LerpAlpha);

	/**
	* 조준하는 방향에 따라 캐릭터의 회전 방향을 결정합니다.
	* @param AimDirection - 조준 방향 벡터입니다.
	* @return NewYaw - 목표로 하는 Yaw값입니다.
	*/
	UFUNCTION(BlueprintCallable)
	float DecideDirectionOfRotation(FVector AimDirection);

	/**
	* 공격을 시작합니다.
	* @param NewCollisionPresetName - 투사체가 사용할 충돌 프리셋 이름
	*/
	UFUNCTION(BlueprintCallable)
	void StartFiring(FName NewCollisionPresetName);

	int32 GetStep() { return Step; };

	/**
	* 액션 포인트를 사용합니다.
	* @param PointToUse - 사용할 액션 포인트 값
	*/
	UFUNCTION()
	void UseActionPoint(int32 PointToUse);

	/** 액션 포인트를 회복한다. */
	void RestoreActionPoint();

	/** 주변을 스캔해 공격 가능한 적들에 대한 정보를 갱신합니다. */
	void ScanEnemy();

	TArray<FAimingInfo>& GetAimingInfo() { return AimingInfo; };

	FAimingInfo GetSelectedAimingInfo() { return SelectedAimingInfo; };

	/** 유닛이 사격할때 선택한 Aiming Info */
	UPROPERTY(BlueprintReadOnly)
	FAimingInfo SelectedAimingInfo;

	UFUNCTION(BlueprintPure, BlueprintCallable)
	FVector GetShootingPlace() { return SelectedAimingInfo.StartLocation; };

	TMap<EAction, bool> GetPossibleAction() { return PossibleActionMap; };

	/**
	* Widget으로부터 받아온 Index로 적을 선택, 공격합니다.
	* @param TargetEnemyIndex - AimingInfo Index
	*/
	void AttackEnemyAccoringToIndex(const int32 TargetEnemyIndex);

	/**
	* 파라미터로 받아온 AimingInfo로 공격합니다.
	* @param TargetAimingInfo
	*/
	void AttackEnemyAccrodingToState(const FAimingInfo TargetAimingInfo);

	void StartTrajectory();

	void FinishTrajectory();

	/**
	* HPbar 의 가시성을 제어합니다.
	* @param bVisible - 게임에 HPBar가 보일지 안보일지 여부
	*/
	void SetHealthBarVisibility(const bool bVisible);

	/**
	* 경계중일때 적의 움직임에 반응합니다.
	* @param TargetLocation - 목표 지점
	*/
	void InVigilance(const FVector TargetLocation);

	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable)
	void CoverMoving(FVector TargetLocation);

	/** Cover 상태에서 움직임이 끝난 후 공격을 시작합니다.*/
	UFUNCTION(BlueprintCallable)
	void AttackAfterCoverMoving();

	// AI를 위한 메소드
	UAimingComponent* GetAimingComponent() { return AimingComponent; };

	/** 사격 완료 후 필요한 작업을 수행합니다. */
	UFUNCTION(BlueprintCallable)
	void AfterShooting();

	/**
	* 공격이 끝난 후 델리게이트 실행, Flag 변환
	* @param bExecuteDelegate - Delegate 실행 여부
	*/
	void SetOffAttackState(const bool bExecuteDelegate);

	/** 경계 활동을 멈춥니다.*/
	void StopVisilance();

	/**
	* 적의 이동에 반응하도록 Delegate 바인딩을 합니다.
	* @param OppositeTeamMember - 카메라 위치
	*/
	void BindVigilanceEvent(const TArray<ACustomThirdPerson*> OppositeTeamMember);

	UFUNCTION()
	void WatchOut(const FVector TargetLocation);

	/**
	* 한타일씩 이동합니다.
	* @param Progress - 목표 타일까지의 진행도
	*/
	UFUNCTION()
	void MovingStepByStep(const int32 Progress);

	/**
	* 다음 타일 방향으로 캐릭터를 회전시킵니다..
	* @param NextTileLocation - 다음 타일의 위치
	*/
	void RotateToNextTile(const FVector NextTileLocation);

	/**
	* 다음 목표지점으로 이동시킵니다. (Lerp)
	* @param LerpAlpha
	*/
	UFUNCTION()
	void MoveToNextTarget(const float LerpAlpha);

	/** 목표로 하던 타일에 도착했을때 호출됩니다. */
	UFUNCTION()
	void ArriveNextTile();

	/**
	* 목표 타일로 이동합니다.
	* @param OnTheWay - 경로를 담은 배열
	* @param ActionPointToUse - 사용할 액션 포인트
	*/
	void MoveToTargetTile(TArray<FVector>* OnTheWay , const int32 ActionPointToUse);
	
	void SetSpeed(float Speed) { this->Speed = Speed; };

	UFUNCTION(BlueprintCallable)
	float GetSpeed() { return Speed; }

	/** 다른 유닛에게 활성화를 넘깁니다.*/
	void ExecuteChangePawnDelegate();

	void SetWalkingState(EWalkingState WalkingStateToSet);

	/**
	* 유닛이 안개속에 있는지 여부를 확인합니다.
	* @return 안개속에 있는지 여부
	*/
	bool IsInUnFoggedArea() const;
	
	UFUNCTION(BlueprintCallable)
	void FinishDodge();

	AGun& GetGunReference() { return *GunReference; };

	/**
	* 수류탄 투척을 준비합니다.
	* @param Velocity - 수류탄을 던질 방향 벡터
	*/
	void PrepareThrowGrenade(FVector Velocity);

	EUnitState GetUnitState() { return UnitState; };

	UPROPERTY(BlueprintReadOnly)
	EUnitState UnitState = EUnitState::Idle;

	/** 경계 시작을 알립니다.*/
	void AnnounceVisilance();
	 
	UPROPERTY()
	UHUDComponent* HUDComponent = nullptr;

	/**
	* 현재 메쉬의 머리 위치를 얻어옵니다.
	* @return 현재 메쉬의 머리 월드 좌표
	*/
	FVector GetHeadLocation();

protected:
	/** 현재 이동하는 모습을 결정할 State*/
	EWalkingState WalkingState = EWalkingState::Running;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UFogOfWarComponent* FOWComponent = nullptr;

	UPROPERTY()
	class UUnitHUD* HPBar = nullptr;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	AGun* GunReference;

	/** 유닛이 사망했을때 호출됩니다.*/
	virtual void Dead();

	/** 이동을 종료합니다.*/
	virtual void FinishMoving();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser) override;
private:
	UPROPERTY()
	UAimingComponent* AimingComponent = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	UTrajectoryComponent* TrajectoryComponent = nullptr;

	UPROPERTY()
	USkeletalMeshComponent* SkeletalMesh = nullptr;

	/** 유닛의 팀을 결정하는 플래그 ( 플레이어측 / AI 측 ) */
	UPROPERTY(EditDefaultsOnly)
	bool bTeam = true;

	/** 이동 가능한 걸음 수  */
	UPROPERTY(EditDefaultsOnly)
	int32 Step = 10;


	UPROPERTY()
	class UTimelineComponent* MovingTimeline;

	UPROPERTY()
	UCurveFloat* FloatCurve;

	FVector RelativeCoverLoc = FVector(0,0,0);

	/** 후보로 갖고있는 모든 조준 정보의 배열입니다 */
	TArray<FAimingInfo> AimingInfo;

	//이후에 클래스로 묶일 능력치들
	int32 Aiming = 0.8;

	/** 유닛이 할 수 있는 액션의 Map입니다. */
	TMap<EAction, bool> PossibleActionMap;

	/** 이전 위치 */
	FVector PrevLocation;

	/** 다음 위치 */
	FVector NextLocation;

	/** 경로를 찾아서 이동하는데 필요한 배열 */
	TArray<FVector> PathToTargetTile;

	/** 이동하는데 사용할 액션 포인트 */
	int32 ActionPointForMoving;

	int32 MovementIndex = 0;

	/** 애니메이션에 사용할 속도 값 */
	float Speed = 0;

	/** 이동에 필요한 Timeline을 초기화합니다. */
	void InitializeTimeline();

	/**
	* 엄폐중인 적이 있는지 확인
	* @param TargetEnemyInfo - EAimingFactor / float
	* @return 엄폐쭝인 적이 있는지 여부
	*/
	bool CheckTargetEnemyCoverState(const TMap<EAimingFactor, float>& TargetEnemyInfo);

	/**
	* 치명타율, 회피율을 결정합니다.
	* @param CriticalChance - 치명타율
	* @param DodgeChance - 회피율
	*/
	void DecideShootingChance(OUT float& CriticalChance, OUT float& DodgeChance);

	/** 유닛을 제외한 월드의 TimeDilation을 줄입니다 (유닛 제외 월드에 슬로우 모션 적용) */
	void StartSlowMotion();

	/** 월드의 TimeDilation을 원상태로 복구합니다 (GlobalTimeDilation = 1) */
	UFUNCTION()
	void FinishSlowMotion();

	/**
	* 엄폐중 이동할때 방향을 설정합니다.InformVisilanceSuccess
	* @param TargetLocation - 목표 지점
	*/
	void SetMovingDirectionDuringCover(const FVector TargetLocation);

	/**
	* 엄폐 이동 후 공격합니다.
	* @param TargetAimingInfo - AimingInfo
	*/
	void CoverUpAndAttack(const FAimingInfo TargetAimingInfo);

	/** 적에게 사격합니다.*/
	UFUNCTION(BlueprintCallable)
	void Shoot();

	/** 현재 State에 따라 이동에 관련된 Timeline을 수정합니다. */
	void ChangeTimelineFactor();

	/**
	* 수류탄을 투척합니다.
	* @param Velocity - 수류탄을 던질 방향 벡터
	* @param Grenade - 던질 수류탄 Actor pointer
	*/
	void ThrowGrenade(FVector Velocity, AGrenade* Grenade);

	/**
	* 목표에서 빗나간 각도를 구할때 호출합니다.
	* @return 사격시 잘못된 각도
	*/
	FVector GetWrongDirection();

	/** Mesh를 제외한 컴포넌트들을 제거합니다. */
	void DestroyUnnecessaryComponents();

};
