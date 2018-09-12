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
	* 투사체의 궤적을 그려냅니다.
	* @param bCanCognize 안개안에 위치하고있는지 여부
	*/
	void DrawProjectileTrajectory();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction) override;

	/** 던지는 힘의 최대값 */
	UPROPERTY(EditDefaultsOnly)
	float MaxThrowingPower = 800;

	AActor* ImpactRangeActor = nullptr;

	/** 궤도 추적을 시작합니다. */
	UFUNCTION()
	void StartDraw();

	/** 궤도 추적을 종료합니다. */
	void FinishDraw();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> ImpactRangeBP;

	UFUNCTION()
	void ConfirmedExplosionArea(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

private:
	/** 궤도를 그릴지 안그릴지 제어하는 플래그 */
	bool bIsDrawing = false;

	UPROPERTY(EditDefaultsOnly)
	FVector InitialLocalVelocity = FVector(1 , 0, 1);

	FVector InitialVelocity;
	
	UPROPERTY(EditDefaultsOnly)
	float TimeInterval = 0.05;

	UPROPERTY(EditDefaultsOnly)
	float PathLifeTime = 5;

	/**
	* 투사체의 궤적을 그려냅니다.
	* @param StartLocation 그려낼 궤적의 시작 지점
	* @param InitialVelocity 속도 벡터
	* @param Gravity 중력 벡터
	* @param Time
	* @param Time2
	* @param FirstPoint 궤도를 그릴 첫번째 점 위치
	* @param SecondPoint 궤도를 그릴 두번째 점 위치
	*/
	void GetSegmentAtTime(const FVector StartLocation, const FVector InitialVelocity, const FVector Gravity, const float Time, const float Time2, OUT FVector& FirstPoint, OUT FVector& SecondPoint);

	float CorrectPower(const float Power);

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** 궤도 추적의 도착점에 투사체 폭발 반경을 나타낼 Actor를 생성합니다. */
	void SpawnImapactRangeActor();
};
