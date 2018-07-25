// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Grenade.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
/**
 * 
 */
UCLASS()
class XCOM_API AGrenade : public AStaticMeshActor
{
	GENERATED_BODY()
	
public:
	AGrenade();

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	int8 Damage;

	UPROPERTY(EditDefaultsOnly)
	float ExplosionRadius;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	class UParticleSystem* ExplosionParticle;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	class USoundCue* ExplosionSound;

	void SetGrenadeVelocity(FVector Velocity);
	
	void EnablePhysics();
protected:

private:
	UPROPERTY(EditDefaultsOnly)
	UProjectileMovementComponent* ProjectileComponent;

	UPROPERTY(EditDefaultsOnly)
	USphereComponent* SphereCollision;

	void Explode();
};
