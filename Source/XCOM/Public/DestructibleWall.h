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

	/**
	* 적용되는 엄폐의 종류
	*/
	UPROPERTY(EditDefaultsOnly)
	ECoverInfo CoverInfo = ECoverInfo::FullCover;

	/**
	* 피격에 견딜수 있는 최대 횟수
	*/
	UPROPERTY(EditDefaultsOnly)
	int32 MaxDurability = 6;

	/**
	* 붕괴까지 남은 피격 횟수
	*/
	int32 Durability;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* BoxCollision = nullptr;;

	/**
	* 엄폐하는 유닛을 등록합니다.
	* @param ActorToRegister - 등록할 Unit의 포인터
	*/
	void RegisterUnit(class ACustomThirdPerson* ActorToRegister);

	/**
	* 해당 Unit이 엄폐를 종료했을때 호출되는 메소드입니다.
	* @param ActorToCancel - 엄폐를 종료하는 Unit의 포인터
	*/
	UFUNCTION()
	void CancelCovering(ACustomThirdPerson* ActorToCancel);

private:
	void Destroyed();

	TArray<ACustomThirdPerson*> CoveredUnitArray;

	UDestructibleComponent* DestructibleCompReference;

	/**
	* 붕괴시 호출되는 메소드입니다.
	*/
	UFUNCTION()
	void Fractured(const FVector& HitPoint, const FVector& HitDirection);

	/**
	* 붕괴 시작 단계에서 호출되는 메소드입니다.
	*/
	UFUNCTION()
	void BeginDestroying();

	/**
	* 붕괴 마지막 단계에서 호출되는 메소드로 Actor를 제거합니다.
	*/
	UFUNCTION()
	void FinisihDestroying();
};
