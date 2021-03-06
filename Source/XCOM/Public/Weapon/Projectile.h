// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Projectile.generated.h"

class UProjectileMovementComponent;
/**
 * 
 */
UCLASS()
class XCOM_API AProjectile : public AStaticMeshActor
{
	GENERATED_BODY()
	
	
public:
	AProjectile();

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	UProjectileMovementComponent* ProjectileMovementComponent;

	void SetProjCollisionChannel(FName NewPresetName);

	void SetProjectileSpeed(float Speed);

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	class UParticleSystem* ExplosionParticle;

private: 
	UStaticMeshComponent* Mesh = nullptr;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	void ApplyToDestructibleActor(const FVector HitLocation);

};
