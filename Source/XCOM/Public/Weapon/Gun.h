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

	UFUNCTION(BlueprintCallable)
	void GenerateProjectile();

	FName ProjectileCollisionPresetName;

	UPROPERTY(EditDefaultsOnly)
	int8 Damage = 1;

	int8 GetDamage() { return  Damage; };

	void FireToTarget(AActor* TargetActor);

	void SetShootingResult(bool bAimSuccess, bool bCritical = false);

private:
	UStaticMeshComponent* StaticMeshComponentRef = nullptr;
	
	//0 ~ 5
	int32 FiringRotOrder = 0;

	UFUNCTION()
	void ApplyDamageToTarget(AActor* TargetActor);

	bool bAimSuccess;

	bool bCritical;

	int8 CalculateActualDamage() const;
};

