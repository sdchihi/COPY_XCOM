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
	CurrentHP = Status.MaxHP;

	CurrentMovableStep = Status.Step;

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

void ACustomThirdPerson::RotateCharacter(float NewYaw, float LerpAlpha)
{
	float ActorYaw = GetActorRotation().Yaw;

	SetActorRotation(FRotator(0, FMath::Lerp(ActorYaw, NewYaw, LerpAlpha), 0));
}

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

void ACustomThirdPerson::RestoreActionPoint()
{
	RemainingActionPoint = 2;
	bCanAction = true;
}

void ACustomThirdPerson::ScanEnemy()
{
	AimingInfo.Empty();
	AimingComponent->GetAttackableEnemyInfo(GetActorLocation(), &Status, bIsCovering, CoverDirectionMap, AimingInfo);
};

void ACustomThirdPerson::AttackEnemyAccoringToIndex(const int32 TargetEnemyIndex) 
{
	FAimingInfo TargetAimingInfo = AimingInfo[TargetEnemyIndex];
	AttackEnemyAccrodingToState(TargetAimingInfo);
}

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

void ACustomThirdPerson::Shoot()
{
	FVector AimDirection = SelectedAimingInfo.TargetLocation - GetActorLocation();

	float CriticalChance = SelectedAimingInfo.SumOfCriticalProb();
	float DodgeChance;

	DecideShootingChance(CriticalChance, DodgeChance);

	float AttackSuccessProbability = SelectedAimingInfo.Probability;
	float result = FMath::FRandRange(0, AttackSuccessProbability);

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

void ACustomThirdPerson::DecideShootingChance(OUT float& CriticalChance, OUT float& DodgeChance)
{
	float RemainingChance = SelectedAimingInfo.Probability;

	DodgeChance = Status.Dodge;
	RemainingChance -= DodgeChance;

	CriticalChance = FMath::Clamp(CriticalChance, 0.f, RemainingChance);
}


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

void ACustomThirdPerson::StartSlowMotion()
{
	CustomTimeDilation = 1;
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(),0.3);
	FTimerHandle UnUsedHandle;
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ACustomThirdPerson::FinishSlowMotion);
	GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 0.4, false);	// 0.4 Delay 고정
}


void ACustomThirdPerson::FinishSlowMotion()
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1);
}


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

void ACustomThirdPerson::InVigilance(const FVector TargetLocation)
{
	FAimingInfo* AimingInfoResult = new FAimingInfo();
	bool bDiscover = AimingComponent->GetVigilanceAimingInfo(&Status, bIsCovering, CoverDirectionMap, TargetLocation, *AimingInfoResult);

	if (bDiscover) 
	{
		FAimingQueue::Instance().StartAiming(this, AimingInfoResult);
	}
	else 
	{
		delete AimingInfoResult;
	}
}

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

void ACustomThirdPerson::InformVisilanceSuccess(const FVector StartLocation, const FVector TargetLocation)
{
	if (ChangeViewTargetDelegate.IsBound()) 
	{
		ChangeViewTargetDelegate.Execute(StartLocation, TargetLocation);
	}
}

void ACustomThirdPerson::BindVigilanceEvent(const TArray<ACustomThirdPerson*> OppositeTeamMember)
{
	for (ACustomThirdPerson* SingleEnemyCharacter : OppositeTeamMember)
	{
		SingleEnemyCharacter->UnprotectedMovingDelegate.AddUniqueDynamic(this, &ACustomThirdPerson::WatchOut);
	}
}

void ACustomThirdPerson::WatchOut(const FVector TargetLocation)
{
	InVigilance(TargetLocation);
}


void ACustomThirdPerson::StopVisilance()
{
	AXCOMGameMode* GameMode = Cast<AXCOMGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	
	const TArray<ACustomThirdPerson*> OppositeTeamMember = GameMode->GetTeamMemeber(!GetTeamFlag());
	for (ACustomThirdPerson* SingleEnemyCharacter : OppositeTeamMember)
	{
		SingleEnemyCharacter->UnprotectedMovingDelegate.RemoveDynamic(this, &ACustomThirdPerson::WatchOut);
	}
}

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

	StartMovingDelegate.Broadcast(this);

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

void ACustomThirdPerson::MovingStepByStep(const int32 Progress = 0) 
{ 
	NextLocation = PathToTargetTile[Progress];
	PrevLocation = GetActorLocation();
	NextLocation.Z = PrevLocation.Z;

	RotateToNextTile(NextLocation);
	MovingTimeline->Stop();
	MovingTimeline->PlayFromStart();
}

void ACustomThirdPerson::RotateToNextTile(const FVector NextTileLocation) 
{
	FVector Direction = (NextTileLocation - GetActorLocation());
	Direction.Z = 0;

	SetActorRotation(Direction.Rotation());
}

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

void ACustomThirdPerson::FinishMoving() 
{
	SetSpeed(0);
	UseActionPoint(ActionPointForMoving);
	if (AfterMovingDelegate.IsBound())
	{
		AfterMovingDelegate.Execute(this);
	}
}

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


bool ACustomThirdPerson::IsInUnFoggedArea() const
{
	return FOWComponent->isActorInTerraIncog;
}

void ACustomThirdPerson::FinishDodge()  
{
	UnitState = EUnitState::Idle;
}

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

		SetActorRotation(FRotator(0, Velocity.Rotation().Yaw, 0));

		FTimerHandle UnUsedHandle;
		FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ACustomThirdPerson::ThrowGrenade, Velocity, SpawnedGrenade);
		GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 2, false);

		UseActionPointAfterDelay(5.f, 2);
	}
}

void ACustomThirdPerson::ThrowGrenade(FVector Velocity, AGrenade* Grenade)
{
	Grenade->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Grenade->SetGrenadeVelocity(Velocity);
	UE_LOG(LogTemp, Warning, L"%s 는 수류탄 Velocity", *Velocity.ToString());
}

FVector ACustomThirdPerson::GetWrongDirection()
{
	ACustomThirdPerson* TargetUnit = Cast<ACustomThirdPerson>(SelectedAimingInfo.TargetActor);
	FVector WrongLocation = TargetUnit->GetHeadLocation() + FVector(100, 150, 100);
	FVector WrongDirection = WrongLocation - GetActorLocation();

	return WrongDirection;
}

FVector ACustomThirdPerson::GetHeadLocation() 
{
	return SkeletalMesh->GetBoneLocation(FName("head"), EBoneSpaces::WorldSpace);
}

void ACustomThirdPerson::AnnounceVisilance()  
{
	if (AnnounceDamageDelegate.IsBound())
	{
		AnnounceDamageDelegate.Execute(this, 0, FloatingWidgetState::Visilance);
	}	
}

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

void ACustomThirdPerson::AfterDeadAnimation() 
{
	if (Speed > 0) 
	{
		UseActionPoint(3);
	}
}
