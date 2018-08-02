// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Classes/GameFramework/ProjectileMovementComponent.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "Classes/PhysicsEngine/DestructibleActor.h"
#include "CustomThirdPerson.h"

AProjectile::AProjectile() 
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(FName("ProjectileMovement"));
	ProjectileMovementComponent->MaxSpeed = 2000;
	ProjectileMovementComponent->InitialSpeed = 2000;
	ProjectileMovementComponent->ProjectileGravityScale = 0;

	Mesh = Cast<UStaticMeshComponent>(RootComponent);
	if (Mesh) {
		Mesh->SetEnableGravity(false);
		Mesh->SetSimulatePhysics(true);
		Mesh->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
		Mesh->SetNotifyRigidBodyCollision(true); // 확인 필요   Hit Event 발생 여부
	}
}

void AProjectile::BeginPlay() 
{

}

void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit) 
{
	if (OtherActor) 
	{
		if (Cast<ADestructibleActor>(OtherActor))
		{
			//ApplyToDestructibleActor(Hit.Location);
		}
	}
	UParticleSystemComponent* TempExplosion = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionParticle, GetActorTransform());

	Destroy();
}

void AProjectile::ApplyToDestructibleActor(const FVector HitLocation)
{
	TSubclassOf<UDamageType> DamageType;
	UGameplayStatics::ApplyRadialDamage(
		GetWorld(),
		1,
		HitLocation,
		40,
		DamageType,
		TArray<AActor*>()
		);
	//TODO - Particle 효과 삽입

	Destroy();
}


void AProjectile::SetProjCollisionChannel(FName NewPresetName)
{
	if (Mesh) 
	{
		//Mesh->SetCollisionObjectType(NewCollisionChannel);
		Mesh->SetCollisionProfileName(NewPresetName);
	}
}

void AProjectile::SetProjectileSpeed(float Speed)
{
	ProjectileMovementComponent->InitialSpeed = Speed;
}
