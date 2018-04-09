// Fill out your copyright notice in the Description page of Project Settings.

#include "TrajectoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

UTrajectoryComponent::UTrajectoryComponent() 
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UTrajectoryComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bIsDrawing) 
	{
		DrawProjectileTrajectory();
	}
};



void UTrajectoryComponent::DrawProjectileTrajectory() 
{
	FVector ActorLocation = GetOwner()->GetActorLocation();;
	APlayerController* PlayerController= UGameplayStatics::GetPlayerController(GetWorld(), 0);
	FHitResult MousePointHitResult;
	PlayerController->GetHitResultUnderCursor(ECollisionChannel::ECC_MAX, true, MousePointHitResult);

	float DistanceFromCharToPoint = FVector::Distance(MousePointHitResult.ImpactPoint , ActorLocation);
	float Power = FMath::Clamp(DistanceFromCharToPoint , 0.f, MaxThrowingPower);
	float CorrectedPower = CorrectPower(Power);
	UE_LOG(LogTemp, Warning, L"거리 : %f   힘 : %f ", DistanceFromCharToPoint, CorrectedPower);
	InitialLocalVelocity = FVector(Power, 0, Power);
	//수정 필요

	FRotator RotationToTarget = (MousePointHitResult.ImpactPoint - ActorLocation).Rotation();
	InitialVelocity = FTransform(RotationToTarget, ActorLocation).TransformVector(InitialLocalVelocity);

	int32 PointCount= FMath::Floor(PathLifeTime / TimeInterval);

	FVector GravityVector = FVector(0, 0, -980.0);
	for (int i = 0; i < PointCount; i++) 
	{
		float Time1 = i * TimeInterval;
		float Time2 = (i + 1) * TimeInterval;

		FVector FirstPoint;
		FVector SecondPoint;
		GetSegmentAtTime(ActorLocation, InitialVelocity, GravityVector, Time1, Time2, FirstPoint, SecondPoint);
		
		FHitResult HitResult;
		TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjects;
		TraceObjects.Add(UEngineTypes::ConvertToObjectType(ECC_Visibility));//add your object types here.
		UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), FirstPoint, SecondPoint, TraceObjects, true, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, HitResult, true);
	}
}

void UTrajectoryComponent::GetSegmentAtTime(const FVector StartLocation, const FVector InitialVelocity, const FVector Gravity, const float Time, const float Time2, OUT FVector& FirstPoint, OUT FVector& SecondPoint)
{
	// 시간에 따른 투사체의 위치 
	// ProjectileLocation = StartLoc + InitialVelocity * Time + ( Time * Time * 0.5 ) * Gravity 
	FVector CorrectedVelocity = InitialVelocity * Time;
	FVector GravityAppliedValue =  Gravity * (Time * Time * 0.5);
	FirstPoint = StartLocation + CorrectedVelocity + GravityAppliedValue;

	CorrectedVelocity = InitialVelocity * Time2;
	GravityAppliedValue = Gravity * (Time2 * Time2 * 0.5);
	SecondPoint = StartLocation + CorrectedVelocity + GravityAppliedValue;
};

void UTrajectoryComponent::StartDraw() 
{
	bIsDrawing = true;
};

void UTrajectoryComponent::FinishDraw() 
{
	bIsDrawing = false;
};

float UTrajectoryComponent::CorrectPower(const float Power)
{
	float Multilplier = FMath::DegreesToRadians(90.f / MaxThrowingPower *Power);
	float CorrectedPower = (FMath::Cos(Multilplier)*0.2 + 1) *Power;
	//UE_LOG(LogTemp, Warning, L"corrected : %f ", (FMath::Cos(Multilplier)*0.2));
	return CorrectedPower;
}
