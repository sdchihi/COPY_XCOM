// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomThirdPerson.h"
#include "Gun.h"
#include "Classes/Camera/CameraComponent.h"
#include "Classes/Components/SkeletalMeshComponent.h"
#include "Classes/GameFramework/SpringArmComponent.h"
#include "Classes/GameFramework/CharacterMovementComponent.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "TrajectoryComponent.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "Classes/Components/CapsuleComponent.h"
#include "FAimingQueue.h"
#include "PawnController.h"
#include "XCOMGameMode.h"
#include "Public/UObject/ConstructorHelpers.h"
#include "Components/TimelineComponent.h"
#include "Classes/Curves/CurveFloat.h"
#include "FogOfWarComponent.h"
#include "Classes/Animation/AnimMontage.h"
#include "Animation/AnimEnums.h"
#include "Kismet/KismetMathLibrary.h"
#include "FloatingWidget.h"
#include "Grenade.h"
#include "HUDComponent.h"
#include "UnitHUD.h"

// Sets default values
ACustomThirdPerson::ACustomThirdPerson()
{
	PrimaryActorTick.bCanEverTick = true;

	AimingComponent = CreateDefaultSubobject<UAimingComponent>(TEXT("AimingComponent"));
	TrajectoryComponent = CreateDefaultSubobject<UTrajectoryComponent>(TEXT("TrajectoryComponent"));
	FOWComponent = CreateDefaultSubobject<UFogOfWarComponent>(TEXT("FogOfWarComponent"));
}

void ACustomThirdPerson::BeginPlay()
{
	Super::BeginPlay();
	CurrentHP = MaxHP;

	CurrentMovableStep = Step;

	CoverDirectionMap.Add(EDirection::East, ECoverInfo::None);
	CoverDirectionMap.Add(EDirection::West, ECoverInfo::None);
	CoverDirectionMap.Add(EDirection::North, ECoverInfo::None);
	CoverDirectionMap.Add(EDirection::South, ECoverInfo::None);

	PossibleActionMap.Add(EAction::Attack, true);
	PossibleActionMap.Add(EAction::Grenade, true);
	PossibleActionMap.Add(EAction::Ambush, true);
	PossibleActionMap.Add(EAction::Vigilance, true);
	
	GunReference = GetWorld()->SpawnActor<AGun>(
		GunBlueprint,
		FVector(0,0,0),
		FRotator(0, 0, 0)
		);
	GunReference->SetOwner(this);

	USkeletalMeshComponent* Mesh = FindComponentByClass<USkeletalMeshComponent>();
	if (Mesh) 
	{
		GunReference->AttachToComponent(Cast<USceneComponent>(Mesh), FAttachmentTransformRules::KeepRelativeTransform,  FName(L"Gun"));
	}
	HUDComponent = FindComponentByClass<UHUDComponent>();
	HPBar = Cast<UUnitHUD>(HUDComponent->GetHPBarWidgetObj());

	SkeletalMesh = FindComponentByClass<USkeletalMeshComponent>();
	InitializeTimeline();
}

void ACustomThirdPerson::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACustomThirdPerson::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACustomThirdPerson::ClearCoverDirectionInfo() 
{
	// Add 메소드가 중복된 Key값에대해선 갱신 역할을 함.
	CoverDirectionMap.Add(EDirection::East, ECoverInfo::None);
	CoverDirectionMap.Add(EDirection::West, ECoverInfo::None);
	CoverDirectionMap.Add(EDirection::North, ECoverInfo::None);
	CoverDirectionMap.Add(EDirection::South, ECoverInfo::None);
	MovingDirectionDuringCover = EDirection::None;
	bIsCovering = false;
}

/**
* 엄폐시 올바른 애니메이션을 위해 벽을 향해 회전합니다.
*/
void ACustomThirdPerson::RotateTowardWall() {

	for (auto CoverDirection : CoverDirectionMap)
	{
		if (CoverDirection.Value != ECoverInfo::None)
		{
			FRotator Direction;
			switch (CoverDirection.Key)
			{
			case EDirection::East:
				Direction = FRotator(0, 0, 0);
				break;
			case EDirection::West:
				Direction = FRotator(0, 180, 0);
				break;
			case EDirection::North:
				Direction = FRotator(0, 90, 0);
				break;
			case EDirection::South:
				Direction = FRotator(0, 270, 0);
				break;
			}
			SetActorRotation(Direction);
		}
	}
}

/**
* 공격이 끝난 후 델리게이트 실행, Flag 변환
* @param bExecuteDelegate - Delegate 실행 여부 
*/
void ACustomThirdPerson::SetOffAttackState(const bool bExecuteDelegate) {
	UnitState = EUnitState::Idle;

	if (bExecuteDelegate)
	{
		ExecuteChangePawnDelegate();
	}

	if (bIsCovering) {
		SetActorLocation(PrevLocation);
		RotateTowardWall();
	}
	if (!bInVisilance) 
	{
		UseActionPoint(2);
	}
	else 
	{
		bInVisilance = false;
	}
}

/**
* 다른 유닛에게 활성화를 넘깁니다.
*/
void ACustomThirdPerson::ExecuteChangePawnDelegate()
{
	if (ChangePlayerPawnDelegate.IsBound())
	{
		ChangePlayerPawnDelegate.Execute();
	}
	else
	{
		UE_LOG(LogTemp, Warning, L"Unbound");
	}

}

/**
* 블루프린트에서 호출될 수 있는 메소드로 일정 시간내에 캐릭터를 목표 방향으로 회전시킵니다. (Timeline과 연계해서 사용)
* @param NewYaw - 목표로 하는 Yaw값입니다.
* @param LerpAlpha - Lerp에 사용될 Alpha값입니다.
*/
void ACustomThirdPerson::RotateCharacter(float NewYaw, float LerpAlpha)
{
	float ActorYaw = GetActorRotation().Yaw;

	SetActorRotation(FRotator(0, FMath::Lerp(ActorYaw, NewYaw, LerpAlpha), 0));
}

/**
* 조준하는 방향에 따라 캐릭터의 회전 방향을 결정합니다.
* @param AimDirection - 조준 방향 벡터입니다.
* @return NewYaw - 목표로 하는 Yaw값입니다.
*/
float ACustomThirdPerson::DecideDirectionOfRotation(FVector AimDirection)
{
	float ActorYaw = GetActorRotation().Yaw;
	float AimYaw = AimDirection.Rotation().Yaw;
	float DeltaYaw = AimYaw - ActorYaw;
	float NewYaw;
	
	if (!bIsCovering) 
	{
		if (FMath::Abs(DeltaYaw) <= 180)
		{
			PlayAnimMontage(LeftTurnMontage);
		}
		else
		{
			PlayAnimMontage(RightTurnMontage);
		}
	}
	NewYaw = AimYaw;

	return NewYaw;
}

/**
* 공격을 시작합니다.
* @param NewCollisionPresetName - 투사체가 사용할 충돌 프리셋 이름
*/
void ACustomThirdPerson::StartFiring(FName NewCollisionPresetName)
{
	UnitState = EUnitState::Attack;
	if (StartShootingDelegate.IsBound())
	{
		if (SelectedAimingInfo.TargetActor)
		{
			StartShootingDelegate.Execute(SelectedAimingInfo.TargetActor, false);
		}
	}

	GunReference->ProjectileCollisionPresetName = NewCollisionPresetName;
	GunReference->FireToTarget(SelectedAimingInfo.TargetActor);
}

/**
* 액션 포인트를 사용합니다.
* @param PointToUse - 사용할 액션 포인트 값
*/
void ACustomThirdPerson::UseActionPoint(int32 PointToUse) 
{
	RemainingActionPoint -= PointToUse;
	UE_LOG(LogTemp, Warning, L"Use %d Action Point  --  Remaining : %d", PointToUse, RemainingActionPoint);
	if (RemainingActionPoint <=0) 
	{
		bCanAction = false;
		if (AfterActionDelegate.IsBound())
		{
			AfterActionDelegate.Broadcast(bTeam);
		}
	}
}

float ACustomThirdPerson::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage < 1) 
	{
		ActualDamage = 0;
	}

	AGun* EnemyGun = Cast<AGun>(DamageCauser);
	FloatingWidgetState State;
	if (EnemyGun) 
	{
		State = EnemyGun->IsCriticalAttack() ? FloatingWidgetState::Critical : FloatingWidgetState::Damaged;
	}
	CurrentHP -= ActualDamage;

	if (CurrentHP <= 0) // 사망처리
	{
		float NewYaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), DamageCauser->GetActorLocation()).Yaw;
		SetActorRotation(FRotator(0, NewYaw, 0));
		Dead();

		if (DeadCamDelegate.IsBound()) 
		{
			DeadCamDelegate.Execute(this);
			//PlayAnimMontage(TestDeadMontage);
			StartSlowMotion();
		}
	}
	else if ( ActualDamage == 0)
	{
		UnitState = EUnitState::Dodge;
		State = FloatingWidgetState::Dodge;
	}
	else 
	{
		UnitState = EUnitState::GetHit;
		HPBar->ReduceHP(ActualDamage);
	}

	if (AnnounceDamageDelegate.IsBound())
	{
		AnnounceDamageDelegate.Execute(this, ActualDamage, State);
	}	

	return ActualDamage;
}


/**
* 액션 포인트를 회복한다.
*/
void ACustomThirdPerson::RestoreActionPoint()
{
	RemainingActionPoint = 2;
	bCanAction = true;
}

/**
* 주변을 스캔해 공격 가능한 적들에 대한 정보를 갱신합니다.
*/
void ACustomThirdPerson::ScanEnemy()
{
	AimingInfo.Empty();
	AimingComponent->GetAttackableEnemyInfo(GetActorLocation(), AttackRadius, bIsCovering, CoverDirectionMap, AimingInfo);
};
 
/**
* Widget으로부터 받아온 Index로 적을 선택, 공격합니다.
* @param TargetEnemyIndex - AimingInfo Index
*/
void ACustomThirdPerson::AttackEnemyAccoringToIndex(const int32 TargetEnemyIndex) 
{
	FAimingInfo TargetAimingInfo = AimingInfo[TargetEnemyIndex];
	AttackEnemyAccrodingToState(TargetAimingInfo);
}

/**
* 파라미터로 받아온 AimingInfo로 공격합니다.
* @param TargetAimingInfo
*/
void ACustomThirdPerson::AttackEnemyAccrodingToState(const FAimingInfo TargetAimingInfo)
{
	SelectedAimingInfo = TargetAimingInfo;
	if (StartActionDelegate.IsBound()) 
	{
		StartActionDelegate.Execute();
	}
	if (bIsCovering) 
	{
		CoverUpAndAttack(SelectedAimingInfo);
	}
	else 
	{
		Shoot();
	}
}

/**
* 엄폐 이동 후 공격합니다.
* @param TargetAimingInfo - AimingInfo
*/
void ACustomThirdPerson::CoverUpAndAttack(const FAimingInfo TargetAimingInfo) 
{
	UnitState = EUnitState::ReadyToAttack;

	PrevLocation = GetActorLocation();
	bool bHaveToMove = !TargetAimingInfo.StartLocation.Equals(GetActorLocation());
	if (bHaveToMove)
	{
		SetMovingDirectionDuringCover(TargetAimingInfo.StartLocation);
		//CoverMoving(TargetAimingInfo.StartLocation);
	}
	else 
	{
		MovingDirectionDuringCover = EDirection::None;
	}
}

void ACustomThirdPerson::AttackAfterCoverMoving() 
{
	Shoot();
	MovingDirectionDuringCover = EDirection::None;
}

/**
* 적에게 사격합니다.
*/
void ACustomThirdPerson::Shoot()
{
	FVector AimDirection = SelectedAimingInfo.TargetLocation - GetActorLocation();

	float CriticalChance = SelectedAimingInfo.SumOfCriticalProb();
	float DodgeChance;

	DecideShootingChance(CriticalChance, DodgeChance);

	float AttackSuccessProbability = SelectedAimingInfo.Probability;
	float result = FMath::FRandRange(0, AttackSuccessProbability);
	//성공

	UE_LOG(LogTemp, Warning, L"셔플 결과  : %f ,  Prob 값 :  %f , Critical 값 : %f  Dodge 값 : %f", result, AttackSuccessProbability, CriticalChance, DodgeChance);

	if (result <= AttackSuccessProbability)
	{
		float DodgePie = AttackSuccessProbability - DodgeChance;
		if (DodgePie <= result) 
		{
			AimAt(GetWrongDirection(), FName(L"ProjectileToWall"));
			UE_LOG(LogTemp, Warning, L"공격 실패 : Dodge");
			GunReference->SetShootingResult(false);
		}
		else if (result < CriticalChance) 
		{
			AimAt(AimDirection, FName(L"ProjectileToChar"));
			UE_LOG(LogTemp, Warning, L"공격 성공 : Critical Attack");
			GunReference->SetShootingResult(true, true);
		}
		else 
		{
			AimAt(AimDirection, FName(L"ProjectileToChar"));
			UE_LOG(LogTemp, Warning, L"공격 성공 : Normal Attack");
			GunReference->SetShootingResult(true, false);
		}
	}
	else //실패
	{
		if (CheckTargetEnemyCoverState(SelectedAimingInfo.Factor))
		{
			float RandomValue = FMath::FRandRange(0, 1);
			if (RandomValue < 0.5)
			{
				AimAt(GetWrongDirection(), FName(L"ProjectileToWall"));
				UE_LOG(LogTemp, Warning, L"공격 실패 : (엄폐) 허공향해 사격");
			}
			else
			{
				AimAt(GetWrongDirection(), FName(L"ProjectileToWall"));
				UE_LOG(LogTemp, Warning, L"공격 실패 : (엄폐)벽을향해 사격");
			}
		}
		else
		{
			AimAt(GetWrongDirection(), FName(L"ProjectileToWall"));
			UE_LOG(LogTemp, Warning, L"공격 실패 : 허공향해 사격");
		}
		GunReference->SetShootingResult(false);
	}

	float DistanceToTarget = GetDistanceTo(SelectedAimingInfo.TargetActor);
	if (DistanceToTarget > 700.f)
	{
		if (ReadyToAttackDelegate.IsBound())
		{
			ReadyToAttackDelegate.Execute(this, AimDirection);
		}
	}
}


/**
* 치명타율, 회피율을 결정합니다.
* @param CriticalChance - 치명타율
* @param DodgeChance - 회피율
*/
void ACustomThirdPerson::DecideShootingChance(OUT float& CriticalChance, OUT float& DodgeChance)
{
	float RemainingChance = SelectedAimingInfo.Probability;

	//DodgeChance = SelectedAimingInfo->TargetActor->GetDodge();
	DodgeChance = 0.08; // Temp value
	RemainingChance -= DodgeChance;

	CriticalChance = FMath::Clamp(CriticalChance, 0.f, RemainingChance);
}

/**
* 엄폐중인 적이 있는지 확인
* @param TargetEnemyInfo - EAimingFactor / float 
* @return 엄폐쭝인 적이 있는지 여부
*/
bool ACustomThirdPerson::CheckTargetEnemyCoverState(const TMap<EAimingFactor, float>& TargetEnemyInfo)
{
	for (auto SingleEnemyInfo : TargetEnemyInfo) 
	{
		switch (SingleEnemyInfo.Key) 
		{
		case EAimingFactor::FullCover:
		case EAimingFactor::HalfCover:
			if (SingleEnemyInfo.Value != 0) 
			{
				return true;
			}
		}
	}

	return false;
}

void ACustomThirdPerson::StartTrajectory() 
{
	if (TrajectoryComponent)
	{
		TrajectoryComponent->StartDraw();
	}
}

void ACustomThirdPerson::FinishTrajectory()
{
	if (TrajectoryComponent) 
	{
		TrajectoryComponent->FinishDraw();
	}
}

/**
* HPbar 의 가시성을 제어합니다.
* @param bVisible - 게임에 HPBar가 보일지 안보일지 여부
*/
void ACustomThirdPerson::SetHealthBarVisibility(const bool bVisible) 
{
	if (HUDComponent)
	{
		HUDComponent->SetWidgetVisibility(bVisible);
	}
	else 
	{
		UE_LOG(LogTemp, Warning, L" 없음");
	}
}

/**
* 유닛을 제외한 월드의 TimeDilation을 줄입니다 (유닛 제외 월드에 슬로우 모션 적용)
*/
void ACustomThirdPerson::StartSlowMotion()
{
	CustomTimeDilation = 1;
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(),0.3);
	FTimerHandle UnUsedHandle;
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ACustomThirdPerson::FinishSlowMotion);
	GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 0.4, false);	// 0.4 Delay 고정
}

/**
* 월드의 TimeDilation을 원상태로 복구합니다 (GlobalTimeDilation = 1)
*/
void ACustomThirdPerson::FinishSlowMotion()
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1);
}

/**
* 사격 완료 후 필요한 작업을 수행합니다.
*/
void ACustomThirdPerson::AfterShooting() 
{
	if (bInVisilance == true)
	{
		SetOffAttackState(false);
		FAimingQueue::Instance().NextTask();
	}
	else 
	{
		FTimerHandle UnUsedHandle;
		FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ACustomThirdPerson::SetOffAttackState, true);
		GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 3, false);
	}
}

/**
* 경계중일때 적의 움직임에 반응합니다.
* @param TargetLocation - 목표 지점
*/
void ACustomThirdPerson::InVigilance(const FVector TargetLocation)
{
	FAimingInfo* AimingInfoResult = new FAimingInfo();
	bool bDiscover = AimingComponent->GetVigilanceAimingInfo(AttackRadius, bIsCovering, CoverDirectionMap, TargetLocation, *AimingInfoResult);

	if (bDiscover) 
	{
		FAimingQueue::Instance().StartAiming(this, AimingInfoResult);
	}
	else 
	{
		delete AimingInfoResult;
	}
}

/**
* 엄폐중 이동할때 방향을 설정합니다.
* @param TargetLocation - 목표 지점
*/
void ACustomThirdPerson::SetMovingDirectionDuringCover(const FVector TargetLocation) 
{
	FVector ForwardVector = GetActorForwardVector().GetSafeNormal();
	FVector LeftVector = FVector::CrossProduct(ForwardVector, FVector(0, 0, 1));
	FVector DirectionToTarget = (TargetLocation - GetActorLocation()).GetSafeNormal();

	if (LeftVector.Equals(DirectionToTarget))
	{
		MovingDirectionDuringCover = EDirection::West;
	}
	else 
	{
		MovingDirectionDuringCover = EDirection::East;
	}
}

void ACustomThirdPerson::UnderGuard() 
{
	this->CustomTimeDilation = 0;
}

/**
* 경계 중 적을 발견했을때 카메라를 변경시키는 Delegate 함수를 호출합니다.
* @param StartLocation - 카메라 위치
* @param TargetLocation - 목표 지점
*/
void ACustomThirdPerson::InformVisilanceSuccess(const FVector StartLocation, const FVector TargetLocation)
{
	if (ChangeViewTargetDelegate.IsBound()) 
	{
		ChangeViewTargetDelegate.Execute(StartLocation, TargetLocation);
	}
}

/**
* 적의 이동에 반응하도록 Delegate 바인딩을 합니다.
* @param OppositeTeamMember - 카메라 위치
*/
void ACustomThirdPerson::BindVigilanceEvent(const TArray<ACustomThirdPerson*> OppositeTeamMember)
{
	for (ACustomThirdPerson* SingleEnemyCharacter : OppositeTeamMember)
	{
		SingleEnemyCharacter->UnprotectedMovingDelegate.AddUniqueDynamic(this, &ACustomThirdPerson::WatchOut);
	}
}

//TODO
void ACustomThirdPerson::WatchOut(const FVector TargetLocation)
{
	InVigilance(TargetLocation);
}

/**
* 경계 활동을 멈춥니다.
*/
void ACustomThirdPerson::StopVisilance()
{
	AXCOMGameMode* GameMode = Cast<AXCOMGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	
	const TArray<ACustomThirdPerson*> OppositeTeamMember = GameMode->GetTeamMemeber(!GetTeamFlag());
	for (ACustomThirdPerson* SingleEnemyCharacter : OppositeTeamMember)
	{
		SingleEnemyCharacter->UnprotectedMovingDelegate.RemoveDynamic(this, &ACustomThirdPerson::WatchOut);
	}
}

/**
* 목표 타일로 이동합니다.
* @param OnTheWay - 경로를 담은 배열
* @param ActionPointToUse - 사용할 액션 포인트
*/
void ACustomThirdPerson::MoveToTargetTile(TArray<FVector>* OnTheWay, const int32 ActionPointToUse)
{
	PathToTargetTile = *OnTheWay;
	MovementIndex = PathToTargetTile.Num() - 1;
	ActionPointForMoving = ActionPointToUse;

	if (WalkingState == EWalkingState::Running) 
	{
		SetSpeed(400);
	}
	else 
	{
		SetSpeed(200);
	}

	if (bIsCovering)
	{
		ClearCoverDirectionInfo();
		FTimerHandle UnUsedHandle;
		FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ACustomThirdPerson::MovingStepByStep, MovementIndex);
		GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 1.2, false);
	}
	else
	{
		MovingStepByStep(MovementIndex);
	}
}

/**
* 한타일씩 이동합니다.
* @param Progress - 목표 타일까지의 진행도
*/
void ACustomThirdPerson::MovingStepByStep(const int32 Progress = 0) 
{ 
	NextLocation = PathToTargetTile[Progress];
	PrevLocation = GetActorLocation();
	NextLocation.Z = PrevLocation.Z;

	RotateToNextTile(NextLocation);
	MovingTimeline->Stop();
	MovingTimeline->PlayFromStart();
}

/**
* 다음 타일 방향으로 캐릭터를 회전시킵니다..
* @param NextTileLocation - 다음 타일의 위치
*/
void ACustomThirdPerson::RotateToNextTile(const FVector NextTileLocation) 
{
	FVector Direction = (NextTileLocation - GetActorLocation());
	Direction.Z = 0;

	SetActorRotation(Direction.Rotation());
}

/**
* 다음 목표지점으로 이동시킵니다. (Lerp) 
* @param LerpAlpha
*/
void ACustomThirdPerson::MoveToNextTarget(const float LerpAlpha) 
{
	float CorrectedLerpAlpha = LerpAlpha;
	if (WalkingState == EWalkingState::Walk) 
	{
		CorrectedLerpAlpha /= 3;
	}
	FVector CurrentLocation = FMath::Lerp(PrevLocation, NextLocation, CorrectedLerpAlpha);
	SetActorLocation(CurrentLocation);
}

/**
* 목표로 하던 타일에 도착했을때 호출됩니다.
*/
void ACustomThirdPerson::ArriveNextTile()
{
	int32 PathLength = PathToTargetTile.Num();
	if (MovementIndex != 0)
	{
		MovementIndex--;
		MovingStepByStep(MovementIndex);
	}
	else 
	{
		FinishMoving();
	}
	UnprotectedMovingDelegate.Broadcast(GetActorLocation());
}

/**
* 이동에 필요한 Timeline을 초기화합니다.
*/
void ACustomThirdPerson::InitializeTimeline()
{
	FString ImagePath = "/Game/Curve/MovingCurve.MovingCurve.MovingCurve";
	UCurveFloat* Curve = Cast<UCurveFloat>(StaticLoadObject(UCurveFloat::StaticClass(), NULL, *(ImagePath)));

	if (!Curve) 
	{
		return;
	}
	else 
	{
		FloatCurve = Curve;
	}
	FOnTimelineFloat onTimelineCallback;
	FOnTimelineEventStatic onTimelineFinishedCallback;
	if (FloatCurve != NULL)
	{
		MovingTimeline = NewObject<UTimelineComponent>(this, FName("TimelineAnimation"));
		MovingTimeline->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		this->BlueprintCreatedComponents.Add(MovingTimeline); 

		MovingTimeline->SetLooping(false);
		ChangeTimelineFactor();
		MovingTimeline->SetTimelineLengthMode(ETimelineLengthMode::TL_TimelineLength);

		MovingTimeline->SetPlaybackPosition(0.0f, false);

		onTimelineCallback.BindUFunction(this, FName{ TEXT("MoveToNextTarget") });
		onTimelineFinishedCallback.BindUFunction(this, FName{ TEXT("ArriveNextTile") });
		MovingTimeline->AddInterpFloat(FloatCurve, onTimelineCallback);
		MovingTimeline->SetTimelineFinishedFunc(onTimelineFinishedCallback);
		MovingTimeline->RegisterComponent();
	}
}

/**
* 이동을 종료합니다.
*/
void ACustomThirdPerson::FinishMoving() 
{
	SetSpeed(0);
	UseActionPoint(ActionPointForMoving);
	if (AfterMovingDelegate.IsBound())
	{
		AfterMovingDelegate.Execute(this);
	}
}

/**
* 현재 State에 따라 이동에 관련된 Timeline을 수정합니다.
*/
void ACustomThirdPerson::ChangeTimelineFactor() 
{
	if (WalkingState == EWalkingState::Running) 
	{
		MovingTimeline->SetTimelineLength(0.25f);
	}
	else 
	{
		MovingTimeline->SetTimelineLength(0.75f);
	}
}

void ACustomThirdPerson::SetWalkingState(EWalkingState WalkingStateToSet)	
{
	WalkingState = WalkingStateToSet; 
	ChangeTimelineFactor();
};

/**
* 유닛이 안개속에 있는지 여부를 확인합니다.
* @return 안개속에 있는지 여부
*/
bool ACustomThirdPerson::IsInUnFoggedArea() const
{
	return FOWComponent->isActorInTerraIncog;
}

void ACustomThirdPerson::FinishDodge()  
{
	UnitState = EUnitState::Idle;
}

/**
* 유닛이 사망했을때 호출됩니다.
*/
void ACustomThirdPerson::Dead() 
{
	MovingTimeline->Stop();
	UnitState = EUnitState::Dead;
	DestroyUnnecessaryComponents();

	if(UnitDeadDelegate.IsBound())
	{
		UnitDeadDelegate.Broadcast(this);
	}
}

/**
* Mesh를 제외한 컴포넌트들을 제거합니다.
*/
void ACustomThirdPerson::DestroyUnnecessaryComponents() 
{
	UCapsuleComponent* RootCollision = Cast<UCapsuleComponent>(GetRootComponent());
	RootCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HPBar->DestroyHPBar();
	HPBar->RemoveFromParent();
	HPBar->SetVisibility(ESlateVisibility::Hidden);
	FOWComponent->DestroyComponent();
	AimingComponent->DestroyComponent();
	TrajectoryComponent->DestroyComponent();
}

/**
* 수류탄 투척을 준비합니다.
* @param Velocity - 수류탄을 던질 방향 벡터
*/
void ACustomThirdPerson::PrepareThrowGrenade(FVector Velocity)
{
	if (GrenadeBlueprint)
	{
		USkeletalMeshComponent* Mesh = FindComponentByClass<USkeletalMeshComponent>();
		AGrenade* SpawnedGrenade =GetWorld()->SpawnActor<AGrenade>(
			GrenadeBlueprint,
			FVector(0, 0, 0),
			FRotator(0, 0, 0)
			);
		SpawnedGrenade->AttachToComponent(Cast<USceneComponent>(Mesh), FAttachmentTransformRules::KeepRelativeTransform, FName(L"Grenade"));

		//float FuncCallDelay = PlayAnimMontage(EmoteMontage);
		SetActorRotation(FRotator(0, Velocity.Rotation().Yaw, 0));

		//임시로 사용 - Montage 연결 후 변경
		FTimerHandle UnUsedHandle;
		FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ACustomThirdPerson::ThrowGrenade, Velocity, SpawnedGrenade);
		GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 2, false);

		UseActionPointAfterDelay(5.f, 2);
	}
}

/**
* 수류탄을 투척합니다.
* @param Velocity - 수류탄을 던질 방향 벡터
* @param Grenade - 던질 수류탄 Actor pointer
*/
void ACustomThirdPerson::ThrowGrenade(FVector Velocity, AGrenade* Grenade)
{
	Grenade->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Grenade->SetGrenadeVelocity(Velocity);
	UE_LOG(LogTemp, Warning, L"%s 는 수류탄 Velocity", *Velocity.ToString());
}

/**
* 목표에서 빗나간 각도를 구할때 호출합니다.
* @return 사격시 잘못된 각도
*/
FVector ACustomThirdPerson::GetWrongDirection()
{
	ACustomThirdPerson* TargetUnit = Cast<ACustomThirdPerson>(SelectedAimingInfo.TargetActor);
	FVector WrongLocation = TargetUnit->GetHeadLocation() + FVector(100, 150, 100);
	FVector WrongDirection = WrongLocation - GetActorLocation();

	return WrongDirection;
}

/**
* 현재 메쉬의 머리 위치를 얻어옵니다.
* @return 현재 메쉬의 머리 월드 좌표
*/
FVector ACustomThirdPerson::GetHeadLocation() 
{
	return SkeletalMesh->GetBoneLocation(FName("head"), EBoneSpaces::WorldSpace);
}

/**
* 경계 시작을 알립니다.
*/
void ACustomThirdPerson::AnnounceVisilance()  
{
	if (AnnounceDamageDelegate.IsBound())
	{
		AnnounceDamageDelegate.Execute(this, 0, FloatingWidgetState::Visilance);
	}	
}

/**
* 일정 시간 경과 후 액션 포인트를 사용합니다.
* @param Time - 지연 시간
* @param Point - 사용할 액션 포인트 값
*/
void ACustomThirdPerson::UseActionPointAfterDelay(float Time, int32 Point)
{
	FTimerHandle UnUsedHandle;
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ACustomThirdPerson::UseActionPoint, Point);
	GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, Time, false);
}


bool ACustomThirdPerson::IsDead() 
{
	return (CurrentHP <= 0);
}

/**
* Dead Animation 종료 후 호출됩니다.
*/
void ACustomThirdPerson::AfterDeadAnimation() 
{
	if (Speed > 0) 
	{
		UseActionPoint(3);  // Turn넘김
	}
}
