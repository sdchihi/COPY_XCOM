// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomThirdPerson.h"
#include "Gun.h"
#include "Classes/Camera/CameraComponent.h"
#include "Classes/Components/SkeletalMeshComponent.h"
#include "Classes/GameFramework/SpringArmComponent.h"
#include "Classes/GameFramework/CharacterMovementComponent.h"
#include "Runtime/Engine/Public/TimerManager.h"

// Sets default values
ACustomThirdPerson::ACustomThirdPerson()
{
	PrimaryActorTick.bCanEverTick = true;

	AimingComponent = CreateDefaultSubobject<UAimingComponent>(TEXT("AimingComponent"));
}

void ACustomThirdPerson::BeginPlay()
{
	Super::BeginPlay();

	CurrentMovableStep = Step;

	CoverDirectionMap.Add(ECoverDirection::East, ECoverInfo::None);
	CoverDirectionMap.Add(ECoverDirection::West, ECoverInfo::None);
	CoverDirectionMap.Add(ECoverDirection::North, ECoverInfo::None);
	CoverDirectionMap.Add(ECoverDirection::South, ECoverInfo::None);
	
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
	CoverDirectionMap.Add(ECoverDirection::East, ECoverInfo::None);
	CoverDirectionMap.Add(ECoverDirection::West, ECoverInfo::None);
	CoverDirectionMap.Add(ECoverDirection::North, ECoverInfo::None);
	CoverDirectionMap.Add(ECoverDirection::South, ECoverInfo::None);

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
			case ECoverDirection::East:
				Direction = FRotator(0, 0, 0);
				break;
			case ECoverDirection::West:
				Direction = FRotator(0, 180, 0);
				break;
			case ECoverDirection::North:
				Direction = FRotator(0, 90, 0);
				break;
			case ECoverDirection::South:
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
void ACustomThirdPerson::SetOffAttackState() {
	bIsAttack = false;
	if (bIsCovering) {
		RotateTowardWall();
	}
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
	bIsAttack = true;
	GunReference->ProjectileCollisionPresetName = NewCollisionPresetName;
	UseActionPoint(2);

	FTimerHandle UnUsedHandle;
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ACustomThirdPerson::SetOffAttackState);
	GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 1.4 + 1.5, false);
}

void ACustomThirdPerson::UseActionPoint(int32 PointToUse) 
{
	RemainingActionPoint -= PointToUse;
	UE_LOG(LogTemp, Warning, L"Use %d Action Point  --  Remaining : %d", PointToUse, RemainingActionPoint);
	if (RemainingActionPoint <=0) 
	{
		bCanAction = false;
		if (CheckTurnDelegate.IsBound())
		{
			CheckTurnDelegate.Execute(bTeam);
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
		UE_LOG(LogTemp, Warning, L"Dead");
	}

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
	AimingComponent->GetAttackableEnemyInfo(AttackRadius, bIsCovering, CoverDirectionMap, AimingInfo);

};