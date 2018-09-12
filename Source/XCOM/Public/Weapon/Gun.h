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

	/** ����ü �ӵ� */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float ProjectileSpeed = 2000.f;

	/** ����ü�� �����Ͽ� ���� �߻��մϴ�.*/
	UFUNCTION(BlueprintCallable)
	void GenerateProjectile();

	/** �浹ü Preset �̸� */
	FName ProjectileCollisionPresetName;

	/** ������ */
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	int8 Damage = 1;

	/** ġ��Ÿ */
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	int8 Critical = 0;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UCameraShake> CameraShake;

	int8 GetDamage() { return  Damage; };

	int8 GetCriticalAbil() { return Critical; };

	/**
	* ��ǥ���� ���� ����մϴ�.
	* @param TargetActor - ��ǥ�� �ϴ� Actor�Դϴ�.
	*/
	void FireToTarget(AActor* TargetActor);

	/**
	* ��� ����� �����մϴ�.
	* @param AimSuccess - ��� ���� ����
	* @param Critical - ġ��Ÿ ����
	*/
	void SetShootingResult(bool AimSuccess, bool Critical = false);

	bool IsCriticalAttack() { return bCritical; };

	void SetOwner(AActor* OwnerToSet) { Owner = OwnerToSet; };
private:
	UStaticMeshComponent* StaticMeshComponentRef = nullptr;
	
	AActor* Owner = nullptr;

	// �ݵ� ������ ���� ����
	int32 FiringRotOrder = 0;

	/** ���� ���� ���� */
	bool bAimSuccess;

	/** ġ��Ÿ ���� */
	bool bCritical;

	/**
	* ��ǥ������ �������� �����մϴ�.
	* @param TargetActor - ��ǥ�� �ϴ� Actor�Դϴ�.
	*/
	UFUNCTION()
	void ApplyDamageToTarget(AActor* TargetActor);

	/** ������ ������ �����մϴ�.*/
	float CalculateActualDamage() const;
};

