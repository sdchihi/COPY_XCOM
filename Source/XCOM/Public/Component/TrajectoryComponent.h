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
	
	void DrawProjectileTrajectory();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction) override;

	UFUNCTION()
	void StartDraw();

	void FinishDraw();

	UPROPERTY(EditDefaultsOnly)
	float MaxThrowingPower = 800;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> ImpactRangeBP;

	AActor* ImpactRangeActor = nullptr;

	UFUNCTION()
	void ConfirmedExplosionArea(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	UFUNCTION()
	void TestFunc(AActor* TouchedActor, FKey ButtonPressed);


private:

	bool bIsDrawing = false;

	UPROPERTY(EditDefaultsOnly)
	FVector InitialLocalVelocity = FVector(1 , 0, 1);

	FVector InitialVelocity;
	
	UPROPERTY(EditDefaultsOnly)
	float TimeInterval = 0.05;


	UPROPERTY(EditDefaultsOnly)
	float PathLifeTime = 5;

	void GetSegmentAtTime(const FVector StartLocation, const FVector InitialVelocity, const FVector Gravity, const float Time, const float Time2, OUT FVector& FirstPoint, OUT FVector& SecondPoint);

	float CorrectPower(const float Power);

	void DealActorInRange();

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);




	void SpawnImapactRangeActor();

};
