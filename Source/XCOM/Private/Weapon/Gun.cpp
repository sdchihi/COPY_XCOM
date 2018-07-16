// Fill out your copyright notice in the Description page of Project Settings.

#include "Gun.h"
#include "Projectile.h"
#include "Classes/Components/ArrowComponent.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "Classes/Kismet/GameplayStatics.h"

AGun::AGun() {
	FirePos = CreateDefaultSubobject<UArrowComponent>(FName("FirePos"));
	FirePos->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}


void AGun::BeginPlay() {
	Super::BeginPlay();
}

void AGun::FireToTarget(AActor* TargetActor) 
{
	FTimerHandle UnUsedHandle;
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &AGun::ApplyDamageToTarget, TargetActor);
	GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 2, false);	// Delay 2 ��
}


void AGun::ApplyDamageToTarget(AActor* TargetActor) 
{
	UE_LOG(LogTemp, Warning, L"���� �˻�");

	TSubclassOf<UDamageType> DamageType;

	UGameplayStatics::ApplyDamage(
		TargetActor,
		CalculateActualDamage(),
		nullptr,
		this,
		DamageType
	);
}

void AGun::SetShootingResult(bool bAimSuccess, bool bCritical)
{
	this->bAimSuccess = bAimSuccess;
	this->bCritical = bCritical;
}


/**
* ����ü�� �����Ͽ� ���� �߻��մϴ�.
*/
void AGun::GenerateProjectile()
{
	FRotator FireRotation[6] = { FRotator(0, 0, 0), FRotator(2, 0, 0),FRotator(0.5, 1.5, 0) ,FRotator(-1, 1, 0), FRotator(-1, -1, 0), FRotator(0.5, -1.5, 0) };
	AProjectile* ProjectileRef = nullptr;
	if (ProejctileBlueprint)
	{
		ProjectileRef = GetWorld()->SpawnActor<AProjectile>(
			ProejctileBlueprint,
			FirePos->GetComponentLocation(),
			FirePos->GetForwardVector().ToOrientationRotator() + FireRotation[FiringRotOrder]
			);
		
		ProjectileRef->SetProjCollisionChannel(ProjectileCollisionPresetName);
		ProjectileRef->SetLifeSpan(2);

	}

	if (FiringRotOrder == 5)
	{
		FiringRotOrder = 0;
	}
	else
	{
		FiringRotOrder++;
	}
}

int8 AGun::CalculateActualDamage() const 
{
	if (bAimSuccess) 
	{
		return 0;
	}

	int8 Devation = Damage / 5;
	int8 ActualDamage = FMath::FRandRange(Damage - Devation, Damage + Devation);

	if (bCritical) 
	{
		ActualDamage = ActualDamage * 2;
	}

	return ActualDamage;
}
