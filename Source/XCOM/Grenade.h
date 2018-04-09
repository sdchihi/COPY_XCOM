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

protected:

private:
	UPROPERTY(EditDefaultsOnly)
	UProjectileMovementComponent* ProjectileComponent;

	UPROPERTY(EditDefaultsOnly)
	USphereComponent* SphereCollision;



	
	
};
