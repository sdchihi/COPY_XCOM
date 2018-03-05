// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/SkeletalMeshActor.h"
#include "Gun.generated.h"

class UStaticMeshComponent;
class UArrowComponent;
class AProjectile;

/**
 * 
 */
UCLASS()
class XCOM_API AGun : public ASkeletalMeshActor
{
	GENERATED_BODY()
	
	
public:
	AGun();

	virtual void BeginPlay() override;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UArrowComponent* FirePos;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AProjectile> ProejctileBlueprint;

	void Fire();


private:

	UStaticMeshComponent* StaticMeshComponentRef;
	
};
