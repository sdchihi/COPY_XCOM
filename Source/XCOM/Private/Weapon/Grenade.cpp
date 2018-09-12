// Fill out your copyright notice in the Description page of Project Settings.

#include "Grenade.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Classes/Kismet/GameplayStatics.h"

AGrenade::AGrenade()
{
	SphereCollision = CreateDefaultSubobject<USphereComponent>(FName(L"Sphere Collision"));
	SetRootComponent(SphereCollision);
	SphereCollision->OnComponentHit.AddDynamic(this, &AGrenade::OnHit);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(FName(L"Static Mesh"));
	Mesh->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
}


void AGrenade::BeginPlay() 
{
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, this, &AGrenade::Explode, 5, false);
}


void AGrenade::Explode() 
{
	TSubclassOf<UDamageType> DamageType;
	UParticleSystemComponent* TempExplosion = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionParticle, GetActorTransform());
	//UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation());

	UGameplayStatics::ApplyRadialDamage(GetWorld(), Damage, GetActorLocation(), ExplosionRadius, DamageType, TArray<AActor*>(), 0, 0, true); 

	Destroy();
}

void AGrenade::SetGrenadeVelocity(FVector Velocity)
{
	SphereCollision->SetSimulatePhysics(true);
	SphereCollision->SetPhysicsLinearVelocity(Velocity);
}

void AGrenade::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Explode();
}

