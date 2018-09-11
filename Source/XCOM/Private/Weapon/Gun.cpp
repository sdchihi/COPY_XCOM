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

/**
* 목표물을 향해 사격합니다.
* @param TargetActor - 목표로 하는 Actor입니다.
*/
void AGun::FireToTarget(AActor* TargetActor) 
{
	float Distance = FVector::Dist2D(Owner->GetActorLocation(), TargetActor->GetActorLocation());
	float Delay = Distance / ProjectileSpeed;

	FTimerHandle UnUsedHandle;
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &AGun::ApplyDamageToTarget, TargetActor);
	GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, Delay, false);	// 거리에 따른 Delay후 데미지 적용
}

/**
* 목표물에게 데미지를 적용합니다.
* @param TargetActor - 목표로 하는 Actor입니다.
*/
void AGun::ApplyDamageToTarget(AActor* TargetActor) 
{
	TSubclassOf<UDamageType> DamageType;

	UGameplayStatics::ApplyDamage(
		TargetActor,
		CalculateActualDamage(),
		nullptr,
		this,
		DamageType
	);
}

/**
* 사격 결과를 결정합니다.
* @param AimSuccess - 사격 명중 여부
* @param Critical - 치명타 여부
*/
void AGun::SetShootingResult(bool AimSuccess, bool Critical)
{
	bAimSuccess = AimSuccess;
	bCritical = Critical;
}

/**
* 투사체를 생성하여 총을 발사합니다.
*/
void AGun::GenerateProjectile()
{
	FRotator FireRotation[6] = { FRotator(0, 0, 0), FRotator(2, 0, 0),FRotator(0.5, 1.5, 0) ,FRotator(-1, 1, 0), FRotator(-1, -1, 0), FRotator(0.5, -1.5, 0) };
	AProjectile* ProjectileRef = nullptr;
	if (ProejctileBlueprint && CameraShake != nullptr)
	{
		GetWorld()->GetFirstPlayerController()->PlayerCameraManager->PlayCameraShake(CameraShake, 1.0f);

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

/**
* 데미지 편차를 적용합니다.
*/
float AGun::CalculateActualDamage() const 
{
	if (!bAimSuccess) 
	{
		return 0.5;
	}

	int8 Devation = Damage / 5;
	int8 ActualDamage = FMath::FRandRange(Damage - Devation, Damage + Devation);

	if (bCritical) 
	{
		ActualDamage = ActualDamage * 2;
	}

	return ActualDamage;
}
