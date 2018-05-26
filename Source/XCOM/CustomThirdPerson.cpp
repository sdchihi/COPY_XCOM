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

// Sets default values
ACustomThirdPerson::ACustomThirdPerson()
{
	PrimaryActorTick.bCanEverTick = true;

	AimingComponent = CreateDefaultSubobject<UAimingComponent>(TEXT("AimingComponent"));
	TrajectoryComponent = CreateDefaultSubobject<UTrajectoryComponent>(TEXT("TrajectoryComponent"));
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
	SetActorLocation(PrevLocation, true);
	UseActionPoint(2);
	if (bIsCovering) {
		RotateTowardWall();
	}

	if (bExecuteDelegate) 
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
}

/**
* �������Ʈ���� ȣ��� �� �ִ� �޼ҵ�� ���� �ð����� ĳ���͸� ��ǥ �������� ȸ����ŵ�ϴ�. (Timeline�� �����ؼ� ���)
* @param AimDirection - ��ǥ ���� �����Դϴ�.
* @param LerpAlpha - Lerp�� ���� Alpha���Դϴ�.
*/
void ACustomThirdPerson::RotateCharacter(FVector AimDirection, float LerpAlpha) 
{
	float CharacterRotatorYaw = GetActorRotation().Yaw;
	float AimAsRotatorYaw = AimDirection.Rotation().Yaw;

	SetActorRotation(FRotator(0,FMath::Lerp(CharacterRotatorYaw, AimAsRotatorYaw, LerpAlpha), 0));
}

void ACustomThirdPerson::StartFiring(FName NewCollisionPresetName)
{
	bIsReadyToAttack = false;
	bIsAttack = true;
	GunReference->ProjectileCollisionPresetName = NewCollisionPresetName;

	
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
	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	CurrentHP -= ActualDamage;
	//if (CurrentHP <= 0) 
	//{
	//	//TODO ��� Event
	//	UCapsuleComponent* ActorCollsion = FindComponentByClass<UCapsuleComponent>();
	//	ActorCollsion->SetCollisionProfileName(FName("Ragdoll"));
	//	if (DeadCamDelegate.IsBound()) 
	//	{
	//		DeadCamDelegate.Execute(GetActorLocation());
	//		SkeletalMesh->SetSimulatePhysics(true);
	//		SkeletalMesh->SetAllBodiesBelowSimulatePhysics(FName("pelvis"), true, true);
	 
	//		StartSlowMotion();

	//	}
	//	
	//	UE_LOG(LogTemp, Warning, L"Dead");
	//}

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
		UE_LOG(LogTemp, Warning, L"������ ������ 1")
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
	if (this->bInVisilance) 
	{
		UE_LOG(LogTemp, Warning, L"VV OO")
	}
	else 
	{
		UE_LOG(LogTemp, Warning, L"VV XX")
	}
	if (bInVisilance == true)
	{
		UE_LOG(LogTemp, Warning, L"VV AfterShooting")
		FOrderlyAimingInfo* TempP = FAimingQueue::GetPending();
		FAimingQueue& Temp2 = FAimingQueue::Instance();
		FAimingQueue::Instance().NextTask();
		SetOffAttackState(false);
	}
	else 
	{
		UE_LOG(LogTemp, Warning, L"VV AfterShooting XXXX")

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
		UE_LOG(LogTemp, Warning, L"VV  %s Watching Enemy!  -       Start Aiming", *GetName());
		FAimingQueue::Instance().StartAiming(this, AimingInfoResult);
		//AttackEnemyAccrodingToState(AimingInfoResult);
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
