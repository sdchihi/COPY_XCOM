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

	

private: 
	UProjectileMovementComponent* ProjectileMovementComponent;
};
