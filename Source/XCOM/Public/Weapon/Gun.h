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

	/** 투사체 속도 */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float ProjectileSpeed = 2000.f;

	/** 투사체를 생성하여 총을 발사합니다.*/
	UFUNCTION(BlueprintCallable)
	void GenerateProjectile();

	/** 충돌체 Preset 이름 */
	FName ProjectileCollisionPresetName;

	/** 데미지 */
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	int8 Damage = 1;

	/** 치명타 */
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	int8 Critical = 0;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UCameraShake> CameraShake;

	int8 GetDamage() { return  Damage; };

	int8 GetCriticalAbil() { return Critical; };

	/**
	* 목표물을 향해 사격합니다.
	* @param TargetActor - 목표로 하는 Actor입니다.
	*/
	void FireToTarget(AActor* TargetActor);

	/**
	* 사격 결과를 결정합니다.
	* @param AimSuccess - 사격 명중 여부
	* @param Critical - 치명타 여부
	*/
	void SetShootingResult(bool AimSuccess, bool Critical = false);

	bool IsCriticalAttack() { return bCritical; };

	void SetOwner(AActor* OwnerToSet) { Owner = OwnerToSet; };
private:
	UStaticMeshComponent* StaticMeshComponentRef = nullptr;
	
	AActor* Owner = nullptr;

	// 반동 구현을 위한 순서
	int32 FiringRotOrder = 0;

	/** 공격 성공 여부 */
	bool bAimSuccess;

	/** 치명타 여부 */
	bool bCritical;

	/**
	* 목표물에게 데미지를 적용합니다.
	* @param TargetActor - 목표로 하는 Actor입니다.
	*/
	UFUNCTION()
	void ApplyDamageToTarget(AActor* TargetActor);

	/** 데미지 편차를 적용합니다.*/
	float CalculateActualDamage() const;
};

