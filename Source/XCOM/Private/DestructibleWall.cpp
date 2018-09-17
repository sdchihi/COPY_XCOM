// Fill out your copyright notice in the Description page of Project Settings.

#include "DestructibleWall.h"
#include "Classes/Components/DestructibleComponent.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "Projectile.h"
#include "Classes/Components/BoxComponent.h"
#include "CustomThirdPerson.h"
#include "Runtime/Engine/Public/TimerManager.h"

ADestructibleWall::ADestructibleWall() 
{
	DestructibleCompReference = FindComponentByClass<UDestructibleComponent>();
	
	BoxCollision = CreateDefaultSubobject<UBoxComponent>(FName("Box Collision"));
	BoxCollision->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

void ADestructibleWall::BeginPlay() {
	Super::BeginPlay();

	if (DestructibleCompReference) {
		//DestructibleCompReference->SetSimulatePhysics(true);
		DestructibleCompReference->OnComponentHit.AddDynamic(this, &ADestructibleWall::OnHit);
		DestructibleCompReference->OnComponentFracture.AddDynamic(this, &ADestructibleWall::Fractured);
		//DestructibleCompReference->SetNotifyRigidBodyCollision(true); // 확인 필요   Hit Event 발생 여부
	}

	FVector Origin;
	FVector BoxExtent;
	GetActorBounds(false, Origin, BoxExtent);
	BoxCollision->SetRelativeLocation(FVector(0, 0,-BoxExtent.Z / 2));

	CoveredUnitArray.Reserve(4);
	switch(CoverInfo) 
	{
	case ECoverInfo::FullCover:
		Durability = MaxDurability;
		break;
	case ECoverInfo::HalfCover:
		Durability = MaxDurability / 2;
		break;
	}
}

void ADestructibleWall::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit) 
{
	TSubclassOf<UDamageType> DamageType;
	FHitResult HitInfo;
	AProjectile* ProjectileActor = Cast<AProjectile>(OtherActor);

	if (ProjectileActor) {
		//UGameplayStatics::ApplyPointDamage(OtherActor, ProjectileActor->GetDamage() , GetActorLocation(), HitInfo, nullptr, this, DamageType);
		//UGameplayStatics::ApplyDamage(this, ProjectileActor->GetDamage(), nullptr, OtherActor, DamageType);
		UE_LOG(LogTemp, Warning, L"Projectile Hit");
	}
	else {
		UE_LOG(LogTemp, Warning, L"Not Projectile Hit");
	}
}

void ADestructibleWall::RegisterUnit(ACustomThirdPerson* ActorToRegister)
{

	if (ActorToRegister) 
	{
		ActorToRegister->StartMovingDelegate.AddUniqueDynamic(this, &ADestructibleWall::CancelCovering);
	}
	CoveredUnitArray.Add(ActorToRegister);
}

void ADestructibleWall::CancelCovering(ACustomThirdPerson* ActorToCancel)
{
	ActorToCancel->StartMovingDelegate.RemoveDynamic(this, &ADestructibleWall::RegisterUnit);
	CoveredUnitArray.RemoveSwap(ActorToCancel);
}

void ADestructibleWall::Destroyed()
{
	for (auto Unit : CoveredUnitArray) 
	{
		Unit->bIsCovering = false;
	}
}


void ADestructibleWall::Fractured(const FVector& HitPoint, const FVector& HitDirection) 
{
	BoxCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DestructibleCompReference->SetCollisionProfileName(FName(L"MinCollision"));
	Destroyed();

	FTimerHandle UnUsedHandle;
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ADestructibleWall::BeginDestroying);
	GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 3, false);
}

void ADestructibleWall::BeginDestroying()
{
	DestructibleCompReference->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FTimerHandle UnUsedHandle;
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ADestructibleWall::FinisihDestroying);
	GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 1, false);	
}

void ADestructibleWall::FinisihDestroying() 
{
	Destroy();
}
