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

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<AProjectile> ProejctileBlueprint;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float ProjectileSpeed = 2000.f;

	UFUNCTION(BlueprintCallable)
	void GenerateProjectile();

	FName ProjectileCollisionPresetName;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	int8 Damage = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	int8 Critical = 0;

	int8 GetDamage() { return  Damage; };

	int8 GetCriticalAbil() { return Critical; };

	void FireToTarget(AActor* TargetActor);

	void SetShootingResult(bool AimSuccess, bool Critical = false);

	bool IsCriticalAttack() { return bCritical; };

	void SetOwner(AActor* OwnerToSet) { Owner = OwnerToSet; };
private:
	UStaticMeshComponent* StaticMeshComponentRef = nullptr;
	
	AActor* Owner = nullptr;

	//0 ~ 5
	int32 FiringRotOrder = 0;

	UFUNCTION()
	void ApplyDamageToTarget(AActor* TargetActor);

	bool bAimSuccess;

	bool bCritical;

	float CalculateActualDamage() const;
};

