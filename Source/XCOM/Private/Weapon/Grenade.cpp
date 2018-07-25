// Fill out your copyright notice in the Description page of Project Settings.

#include "Grenade.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Classes/Kismet/GameplayStatics.h"



AGrenade::AGrenade()
{
	SphereCollision = CreateDefaultSubobject<USphereComponent>(FName(L"Sphere Collision"));
	SphereCollision->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	ProjectileComponent = CreateDefaultSubobject<UProjectileMovementComponent>(FName("Projectile Movement Component"));

	/*ProjectileComponent->InitialSpeed = 0;
	ProjectileComponent->MaxSpeed = 0;
	ProjectileComponent->Velocity = FVector(0, 0, 0);
	*/
	ProjectileComponent->Bounciness = 0.2;
	ProjectileComponent->bInitialVelocityInLocalSpace = false;

	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AGrenade::OnOverlapBegin);
}


void AGrenade::BeginPlay() 
{
	SphereCollision->SetRelativeLocation(FVector(0, 0, 0));
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, this, &AGrenade::Explode, 5, false);
}

void AGrenade::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Explode();
}

void AGrenade::Explode() 
{
	TSubclassOf<UDamageType> DamageType;
	UParticleSystemComponent* TempExplosion = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionParticle, GetActorTransform());
	//UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation());

	UGameplayStatics::ApplyRadialDamage(GetWorld(), Damage, GetActorLocation(), ExplosionRadius, DamageType, TArray<AActor*>(), 0, 0, true); // 감쇄 효과 없는 데미지 적용

	Destroy();
	//AShooterExplosionEffect* const EffectActor = GetWorld()->SpawnActorDeferred<AShooterExplosionEffect>(ExplosionTemplate, SpawnTransform);	 // BP 의 Constructor 를 잠시 블록함 Finsih SpawningActor 가 호출되기 전까지.
	//if (EffectActor)
	//{
	//	EffectActor->SurfaceHit = Impact;
	//	UGameplayStatics::FinishSpawningActor(EffectActor, SpawnTransform);
	//}
}

void AGrenade::SetGrenadeVelocity(FVector Velocity)
{
	ProjectileComponent->SetVelocityInLocalSpace(Velocity);
}

void AGrenade::EnablePhysics() 
{
	UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(GetRootComponent());
	StaticMeshComp->SetSimulatePhysics(true);
	//SphereCollision->SetSimulatePhysics(true);
}
