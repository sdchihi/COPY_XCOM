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
	* ����Ǵ� ������ ����
	*/
	UPROPERTY(EditDefaultsOnly)
	ECoverInfo CoverInfo = ECoverInfo::FullCover;

	/**
	* �ǰݿ� �ߵ��� �ִ� �ִ� Ƚ��
	*/
	UPROPERTY(EditDefaultsOnly)
	int32 MaxDurability = 6;

	/**
	* �ر����� ���� �ǰ� Ƚ��
	*/
	int32 Durability;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* BoxCollision = nullptr;;

	/**
	* �����ϴ� ������ ����մϴ�.
	* @param ActorToRegister - ����� Unit�� ������
	*/
	void RegisterUnit(class ACustomThirdPerson* ActorToRegister);

	/**
	* �ش� Unit�� ���� ���������� ȣ��Ǵ� �޼ҵ��Դϴ�.
	* @param ActorToCancel - ���� �����ϴ� Unit�� ������
	*/
	UFUNCTION()
	void CancelCovering(ACustomThirdPerson* ActorToCancel);

private:
	void Destroyed();

	TArray<ACustomThirdPerson*> CoveredUnitArray;

	UDestructibleComponent* DestructibleCompReference;

	/**
	* �ر��� ȣ��Ǵ� �޼ҵ��Դϴ�.
	*/
	UFUNCTION()
	void Fractured(const FVector& HitPoint, const FVector& HitDirection);

	/**
	* �ر� ���� �ܰ迡�� ȣ��Ǵ� �޼ҵ��Դϴ�.
	*/
	UFUNCTION()
	void BeginDestroying();

	/**
	* �ر� ������ �ܰ迡�� ȣ��Ǵ� �޼ҵ�� Actor�� �����մϴ�.
	*/
	UFUNCTION()
	void FinisihDestroying();
};
