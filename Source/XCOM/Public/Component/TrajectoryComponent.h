// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "TrajectoryComponent.generated.h"

/**
 * 
 */
UCLASS()
class XCOM_API UTrajectoryComponent : public UPrimitiveComponent
{
	GENERATED_BODY()
	
public:
	UTrajectoryComponent();
	
	/**
	* ����ü�� ������ �׷����ϴ�.
	* @param bCanCognize �Ȱ��ȿ� ��ġ�ϰ��ִ��� ����
	*/
	void DrawProjectileTrajectory();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction) override;

	/** ������ ���� �ִ밪 */
	UPROPERTY(EditDefaultsOnly)
	float MaxThrowingPower = 800;

	AActor* ImpactRangeActor = nullptr;

	/** �˵� ������ �����մϴ�. */
	UFUNCTION()
	void StartDraw();

	/** �˵� ������ �����մϴ�. */
	void FinishDraw();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> ImpactRangeBP;

	UFUNCTION()
	void ConfirmedExplosionArea(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

private:
	/** �˵��� �׸��� �ȱ׸��� �����ϴ� �÷��� */
	bool bIsDrawing = false;

	UPROPERTY(EditDefaultsOnly)
	FVector InitialLocalVelocity = FVector(1 , 0, 1);

	FVector InitialVelocity;
	
	UPROPERTY(EditDefaultsOnly)
	float TimeInterval = 0.05;

	UPROPERTY(EditDefaultsOnly)
	float PathLifeTime = 5;

	/**
	* ����ü�� ������ �׷����ϴ�.
	* @param StartLocation �׷��� ������ ���� ����
	* @param InitialVelocity �ӵ� ����
	* @param Gravity �߷� ����
	* @param Time
	* @param Time2
	* @param FirstPoint �˵��� �׸� ù��° �� ��ġ
	* @param SecondPoint �˵��� �׸� �ι�° �� ��ġ
	*/
	void GetSegmentAtTime(const FVector StartLocation, const FVector InitialVelocity, const FVector Gravity, const float Time, const float Time2, OUT FVector& FirstPoint, OUT FVector& SecondPoint);

	float CorrectPower(const float Power);

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** �˵� ������ �������� ����ü ���� �ݰ��� ��Ÿ�� Actor�� �����մϴ�. */
	void SpawnImapactRangeActor();
};
