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
#include "HUDComponent.h"
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

	CurrentMovableStep = Step;

	CoverDirectionMap.Add(EDirection::East, ECoverInfo::None);
	CoverDirectionMap.Add(EDirection::West, ECoverInfo::None);
	CoverDirectionMap.Add(EDirection::North, ECoverInfo::None);
	CoverDirectionMap.Add(EDirection::South, ECoverInfo::None);

	//Todo - ���� ����
	PossibleActionMap.Add(EAction::Attack, true);
	PossibleActionMap.Add(EAction::Grenade, true);
	PossibleActionMap.Add(EAction::Ambush, true);
	PossibleActionMap.Add(EAction::Vigilance, true);

	
	GunReference = GetWorld()->SpawnActor<AGun>(
		GunBlueprint,
		FVector(0,0,0),
		FRotator(0, 0, 0)
		);

	USkeletalMeshComponent* Mesh = FindComponentByClass<USkeletalMeshComponent>();
	if (Mesh) 
	{
		GunReference->AttachToComponent(Cast<USceneComponent>(Mesh), FAttachmentTransformRules::KeepRelativeTransform,  FName(L"Gun"));
	}

	HealthBar = FindComponentByClass<UHUDComponent>();
	SkeletalMesh = FindComponentByClass<USkeletalMeshComponent>();
	InitializeTimeline();
}



void ACustomThirdPerson::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

	
// Called to bind functionality to input
void ACustomThirdPerson::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}


void ACustomThirdPerson::ClearCoverDirectionInfo() 
{
	// Add �޼ҵ尡 �ߺ��� Key�������ؼ� ���� ������ ��.
	CoverDirectionMap.Add(EDirection::East, ECoverInfo::None);
	CoverDirectionMap.Add(EDirection::West, ECoverInfo::None);
	CoverDirectionMap.Add(EDirection::North, ECoverInfo::None);
	CoverDirectionMap.Add(EDirection::South, ECoverInfo::None);
	MovingDirectionDuringCover = EDirection::None;
	bIsCovering = false;
}



/**
* ����� �ùٸ� �ִϸ��̼��� ���� ���� ���� ȸ���մϴ�.
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
* ������ ���� �� ��������Ʈ ����, Flag ��ȯ
*/
void ACustomThirdPerson::SetOffAttackState(const bool bExecuteDelegate) {
	bIsAttack = false;
	SetActorLocation(PrevLocation);
	if (!bInVisilance) 
	{
		UseActionPoint(2);
	}
	else 
	{
		bInVisilance = false;
	}

	if (bIsCovering) {
		RotateTowardWall();
	}

	if (bExecuteDelegate) 
	{
		ExecuteChangePawnDelegate();
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

/**
* �������Ʈ���� ȣ��� �� �ִ� �޼ҵ�� ���� �ð����� ĳ���͸� ��ǥ �������� ȸ����ŵ�ϴ�. (Timeline�� �����ؼ� ���)
* @param NewYaw - ��ǥ�� �ϴ� Yaw���Դϴ�.
* @param LerpAlpha - Lerp�� ���� Alpha���Դϴ�.
*/
void ACustomThirdPerson::RotateCharacter(float NewYaw, float LerpAlpha)
{
	float ActorYaw = GetActorRotation().Yaw;

	SetActorRotation(FRotator(0, FMath::Lerp(ActorYaw, NewYaw, LerpAlpha), 0));
}

/**
* �����ϴ� ���⿡ ���� ĳ������ ȸ�� ������ �����մϴ�.
* @param AimDirection - ���� ���� �����Դϴ�.
* @return NewYaw - ��ǥ�� �ϴ� Yaw���Դϴ�.
*/
float ACustomThirdPerson::DecideDirectionOfRotation(FVector AimDirection)
{
	float ActorYaw = GetActorRotation().Yaw;
	float AimYaw = AimDirection.Rotation().Yaw;
	float DeltaYaw = AimYaw - ActorYaw;

	float NewYaw;
	if ( FMath::Abs(DeltaYaw) <= 180)
	{
		PlayAnimMontage(LeftTurnMontage);
		NewYaw = AimYaw;
	}
	else 
	{
		PlayAnimMontage(RightTurnMontage);
		NewYaw = ActorYaw - (360.f - AimYaw);
	}

	return NewYaw;
}


void ACustomThirdPerson::StartFiring(FName NewCollisionPresetName)
{
	PrevLocation = GetActorLocation();
	if (StartShootingDelegate.IsBound())
	{
		if (SelectedAimingInfo.TargetActor)
		{
			StartShootingDelegate.Execute(SelectedAimingInfo.TargetActor, false);
		}
	}
	bIsReadyToAttack = false;
	bIsAttack = true;

	GunReference->ProjectileCollisionPresetName = NewCollisionPresetName;
	GunReference->FireToTarget(SelectedAimingInfo.TargetActor);
}

void ACustomThirdPerson::UseActionPoint(int32 PointToUse) 
{
	RemainingActionPoint -= PointToUse;
	UE_LOG(LogTemp, Warning, L"Use %d Action Point  --  Remaining : %d", PointToUse, RemainingActionPoint);
	if (RemainingActionPoint <=0) //TODO
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
	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	CurrentHP -= ActualDamage;
	if (CurrentHP <= 0) 
	{
		//TODO ��� Event
		//UCapsuleComponent* ActorCollsion = FindComponentByClass<UCapsuleComponent>();
		//ActorCollsion->SetCollisionProfileName(FName("Ragdoll"));
		if (DeadCamDelegate.IsBound()) 
		{
			DeadCamDelegate.Execute(this);
			/*
			SkeletalMesh->SetSimulatePhysics(true);
			SkeletalMesh->SetAllBodiesBelowSimulatePhysics(FName("pelvis"), true, true);
			*/
			PlayAnimMontage(TestDeadMontage);
			StartSlowMotion();
		}
		
		UE_LOG(LogTemp, Warning, L"Dead");
	}

	UE_LOG(LogTemp, Warning, L"Damaged");

	return ActualDamage;
}

void ACustomThirdPerson::RestoreActionPoint()
{
	bCanAction = true;
	RemainingActionPoint = 2;
}

// �̵� ��, ���� �ٽ� ���ƿ�����
void ACustomThirdPerson::ScanEnemy() 
{
	AimingInfo.Empty();
	AimingComponent->GetAttackableEnemyInfo(GetActorLocation(), AttackRadius, bIsCovering, CoverDirectionMap, AimingInfo);

};
 
void ACustomThirdPerson::AttackEnemyAccoringToIndex(const int32 TargetEnemyIndex) 
{
	FAimingInfo TargetAimingInfo = AimingInfo[TargetEnemyIndex];
	AttackEnemyAccrodingToState(TargetAimingInfo);
}

void ACustomThirdPerson::AttackEnemyAccrodingToState(const FAimingInfo TargetAimingInfo)
{
	SelectedAimingInfo = TargetAimingInfo;
	// ��Ʈ�ѷ��ʿ� .. StartAction
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
	bIsReadyToAttack = true;
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
	bIsReadyToAttack = false;
}


void ACustomThirdPerson::Shoot() 
{
	FVector AimDirection = SelectedAimingInfo.TargetLocation - GetActorLocation();
	float AttackSuccessProbability = SelectedAimingInfo.Probability;
	float RandomValue = FMath::FRandRange(0, 1);

	//����
	if (RandomValue <= AttackSuccessProbability)
	{
		AimAt(AimDirection, FName(L"ProjectileToChar"));

		UE_LOG(LogTemp, Warning, L"Prob : %f,  Rand : %f ", AttackSuccessProbability, RandomValue);
		GunReference->SetShootingResult(true, false);
		/*
		ũ��Ƽ�� ����� 
		GunReference->SetShootingResult(true,false);
		GunReference->SetShootingResult(false, true);
		*/
	}
	else //����
	{
		if (CheckTargetEnemyCoverState(SelectedAimingInfo.Factor))
		{
			//������
			RandomValue = FMath::FRandRange(0, 1);
			if (RandomValue < 0.5) 
			{
				AimAt(AimDirection + FVector(0, 80, 150), FName(L"ProjectileToWall"));
				UE_LOG(LogTemp, Warning, L"���� ���� : (����) ������� ���");
			}
			else 
			{
				AimAt(AimDirection + FVector(0, 80, 150), FName(L"ProjectileToWall"));
				UE_LOG(LogTemp, Warning, L"���� ���� : (����)�������� ���");
			}
		}
		else 
		{
			AimAt(AimDirection + FVector(0, 50, 50), FName(L"ProjectileToWall"));
			UE_LOG(LogTemp, Warning, L"���� ���� : ������� ���");
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
				UE_LOG(LogTemp, Warning, L" ���� ���� : %f ", SingleEnemyInfo.Value);
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
	if (HealthBar) 
	{
		HealthBar->SetWidgetVisibility(bVisible);

	}
	else 
	{
		UE_LOG(LogTemp, Warning, L" ����");
	}
}


void ACustomThirdPerson::StartSlowMotion()
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(),0.3);
	FTimerHandle UnUsedHandle;
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ACustomThirdPerson::FinishSlowMotion);
	GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 0.4, false);	// 0.4 Delay ����
}


void ACustomThirdPerson::FinishSlowMotion()
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1);
}



void ACustomThirdPerson::StartVisiliance() 
{
	//AttackRadius;
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



// ����� �� �߰�
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
	//

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
		GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 1.2, false);	// 0.5 Delay ����
	}
	else
	{
		MovingStepByStep(MovementIndex);
	}
}


void ACustomThirdPerson::MovingStepByStep(const int32 Progress = 0) 
{ 
	NextLocation = PathToTargetTile[Progress];	//���� ������
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
	//ConstructorHelpers::FObjectFinder<UCurveFloat> Curve(TEXT("/Game/Curve/MovingCurve.MovingCurve.MovingCurve"));
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
		MovingTimeline->CreationMethod = EComponentCreationMethod::UserConstructionScript; // Indicate it comes from a blueprint so it gets cleared when we rerun construction scripts
		this->BlueprintCreatedComponents.Add(MovingTimeline); // Add to array so it gets saved

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

