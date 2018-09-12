// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grenade.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
/**
 * 
 */
UCLASS()
class XCOM_API AGrenade : public AActor
{
	GENERATED_BODY()
	
public:
	AGrenade();

	virtual void BeginPlay() override;

	/** Æø¹ß µ¥¹ÌÁö */
	UPROPERTY(EditDefaultsOnly)
	int8 Damage;

	/** Æø¹ß ¹Ý°æ */
	UPROPERTY(EditDefaultsOnly)
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	class UParticleSystem* ExplosionParticle;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	class USoundCue* ExplosionSound;

	void SetGrenadeVelocity(FVector Velocity);

	UPROPERTY(VisibleAnywhere)
	USphereComponent* SphereCollision;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh = nullptr;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

protected:

private:

	/** Æø¹ßÇÕ´Ï´Ù.*/
	void Explode();
};
