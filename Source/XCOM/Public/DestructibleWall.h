// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PhysicsEngine/DestructibleActor.h"
#include "DestructibleWall.generated.h"

class UBoxComponent;

UENUM(BlueprintType)
enum class ECoverInfo : uint8
{
	FullCover,
	HalfCover,
	Unknown,
	None
};

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

	UPROPERTY(EditDefaultsOnly)
	ECoverInfo CoverInfo = ECoverInfo::FullCover;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxDurability = 6;

	int32 Durability;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* BoxCollision = nullptr;;

	void RegisterUnit(class ACustomThirdPerson* ActorToRegister);

	UFUNCTION()
	void CancelCovering(ACustomThirdPerson* ActorToCancel);

private:
	void Destroyed();

	TArray<ACustomThirdPerson*> CoveredUnitArray;

	UDestructibleComponent* DestructibleCompReference;
	
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void Fractured(const FVector& HitPoint, const FVector& HitDirection);

	UFUNCTION()
	void BeginDestroying();

	UFUNCTION()
	void FinisihDestroying();


};
