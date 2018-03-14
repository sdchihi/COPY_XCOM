// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PhysicsEngine/DestructibleActor.h"
#include "DestructibleWall.generated.h"

/**
 * 
 */
UCLASS()
class XCOM_API ADestructibleWall : public ADestructibleActor
{
	GENERATED_BODY()
	
public:
	ADestructibleWall();

	virtual void BeginPlay() override;


private:

	UDestructibleComponent* DestructibleCompReference;
	
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

};
