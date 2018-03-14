// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerPawnInAiming.h"
#include "Classes/Camera/CameraComponent.h"
#include "Classes/Kismet/KismetMathLibrary.h"
#include "Classes/Kismet/KismetSystemLibrary.h"


APlayerPawnInAiming::APlayerPawnInAiming()
{
	PrimaryActorTick.bCanEverTick = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	SetRootComponent(Camera);
}

void APlayerPawnInAiming::BeginPlay()
{
	Super::BeginPlay();
	
}

void APlayerPawnInAiming::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APlayerPawnInAiming::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void APlayerPawnInAiming::SetCameraPositionInAimingSituation(FVector AimingCharLoc, FVector AimedCharLoc)
{
//	PrevPlayerPawnTransform = GetActorTransform();
	FVector StraightLineDirection = (AimedCharLoc - AimingCharLoc).GetSafeNormal();
	// Z축과 StraightLineDirection의 외적
	FVector RightDirection = FVector::CrossProduct(StraightLineDirection, FVector(0, 0, 1));
	FVector NewPawnPosition = AimingCharLoc - (StraightLineDirection * BackWardDistance) + (RightDirection * RightDistance) + FVector(0, 0, UpwardDistance);
	float DistanceBtwChar = FVector::Distance(AimingCharLoc, AimedCharLoc);

	FVector TargetLocation = AimedCharLoc - (StraightLineDirection * (DistanceBtwChar / 2));
	FRotator NewPawnRotation = UKismetMathLibrary::FindLookAtRotation(NewPawnPosition, TargetLocation);
	SetActorLocation(NewPawnPosition);
	SetActorRotation(NewPawnRotation);
};
