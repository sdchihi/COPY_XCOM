// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerPawnInAiming.h"
#include "Classes/Camera/CameraComponent.h"
#include "Classes/Kismet/KismetMathLibrary.h"
#include "Classes/Kismet/KismetSystemLibrary.h"
#include "Classes/Components/SkeletalMeshComponent.h"

APlayerPawnInAiming::APlayerPawnInAiming()
{
	PrimaryActorTick.bCanEverTick = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	SetRootComponent(Camera);
}

void APlayerPawnInAiming::BeginPlay()
{
	Super::BeginPlay();
}

void APlayerPawnInAiming::Tick(float DeltaTime)
{
	if (bCameraMoving) 
	{
		FVector TargetLocation;
		if (bFocusHead)
		{
			TargetLocation = GetActorsHeadLocation();
		}
		else 
		{
			TargetLocation = ActorToFocus->GetActorLocation() + FVector(0, 0, UpwardDistance / 2);
		}
		FVector DeltaMovement = UKismetMathLibrary::VLerp(GetActorLocation(), EndLocation, GetWorld()->GetDeltaSeconds() * 3);
		FRotator NewPawnRotation = UKismetMathLibrary::FindLookAtRotation(DeltaMovement, TargetLocation);

		SetActorRotation(NewPawnRotation);
		SetActorLocation(DeltaMovement);
	}
	Super::Tick(DeltaTime);
}

void APlayerPawnInAiming::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{ 
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APlayerPawnInAiming::SetCameraPositionInAimingSituation(const FVector AimingCharLoc, const FVector AimedCharLoc)
{
	FVector PawnPosition;
	FVector StraightLineDirection = (AimedCharLoc - AimingCharLoc).GetSafeNormal();
	// Z축과 StraightLineDirection의 외적
	FVector RightDirection = FVector::CrossProduct(StraightLineDirection, FVector(0, 0, 1));
	
	FVector NeedCheckingLocationArray[2];
	NeedCheckingLocationArray[0] = AimingCharLoc - (StraightLineDirection * BackWardDistance) + (RightDirection * RightDistance) + FVector(0, 0, UpwardDistance);	//Right shoulder
	NeedCheckingLocationArray[1] = AimingCharLoc - (StraightLineDirection * BackWardDistance) - (RightDirection * RightDistance) + FVector(0, 0, UpwardDistance);	//Left shoulder

	float DistanceBtwChar = FVector::Distance(AimingCharLoc, AimedCharLoc);
	FVector TargetLocation = AimedCharLoc - (StraightLineDirection * (DistanceBtwChar / 2));
	
	bool bFindStartLocation = false;
	for (FVector  NeedCheckingLocation : NeedCheckingLocationArray)
	{
		if (CheckInView(NeedCheckingLocation, TargetLocation)) 
		{
			PawnPosition = NeedCheckingLocation;
			bFindStartLocation = true;
			bNeedToChangeLocation = false;
			break;
		};
	}

	if (!bFindStartLocation) 
	{
		PawnPosition = NeedCheckingLocationArray[0];
		bNeedToChangeLocation = true;
	}

	FRotator NewPawnRotation = UKismetMathLibrary::FindLookAtRotation(PawnPosition, TargetLocation);
	SetActorLocation(PawnPosition);
	SetActorRotation(NewPawnRotation);
};

void APlayerPawnInAiming::SetShootingCam(const FVector AimingCharLoc, const FVector AimedCharLoc)
{
	if (!bNeedToChangeLocation) {return;}

	FVector PawnPosition;

	FVector StraightLineDirection = (AimedCharLoc - AimingCharLoc).GetSafeNormal();
	FVector RightDirection = FVector::CrossProduct(StraightLineDirection, FVector(0, 0, 1));
	float DistanceBtwChar = FVector::Distance(AimingCharLoc, AimedCharLoc);
	FVector HalfwayPoint = AimedCharLoc - (StraightLineDirection * (DistanceBtwChar / 2));

	FVector NeedCheckingLocationArray[2];
	NeedCheckingLocationArray[0] = AimingCharLoc + (StraightLineDirection * BackWardDistance) + (RightDirection * RightDistance) + FVector(0, 0, UpwardDistance);	//정면
	NeedCheckingLocationArray[1] = HalfwayPoint + FVector(0, DistanceBtwChar+200, DistanceBtwChar + 200);

	FVector TargetLocation[2];
	TargetLocation[0] = AimingCharLoc;
	TargetLocation[1] = HalfwayPoint;

	FVector NewPawnPosition;
	FRotator NewPawnRotation;
	if (CheckInView(NeedCheckingLocationArray[0], TargetLocation[0])) 
	{
		NewPawnPosition = NeedCheckingLocationArray[0];
		NewPawnRotation = UKismetMathLibrary::FindLookAtRotation(NewPawnPosition, TargetLocation[0]);
	}
	else 
	{
		NewPawnPosition = NeedCheckingLocationArray[1];
		NewPawnRotation = UKismetMathLibrary::FindLookAtRotation(NewPawnPosition, TargetLocation[1]);
	}

	SetActorLocation(NewPawnPosition);
	SetActorRotation(NewPawnRotation);
};

bool APlayerPawnInAiming::CheckInView(const FVector StartLocation, const FVector TargetLocation) 
{
	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams CollisionParams;
	CollisionParams.TraceTag = TraceTag;

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		TargetLocation,
		ECollisionChannel::ECC_GameTraceChannel11,
		CollisionParams
	);

	if (HitResult.GetActor()) 
	{
		return false;
	}
	else 
	{
		return true;
	}
}

void APlayerPawnInAiming::SetFrontCam(AActor* Actor) 
{
	FVector ActorLocation = Actor->GetActorLocation();
	FVector ActorsForwardVector = Actor->GetActorForwardVector();
	FVector RightDirection = FVector::CrossProduct(ActorsForwardVector, FVector(0, 0, 1));
	FVector NewPawnPosition = ActorLocation + (ActorsForwardVector * 200) + (RightDirection * RightDistance) + FVector(0, 0, UpwardDistance);

	FRotator NewPawnRotation = UKismetMathLibrary::FindLookAtRotation(NewPawnPosition, ActorLocation);
	SetActorLocation(NewPawnPosition);
	SetActorRotation(NewPawnRotation);
};


void APlayerPawnInAiming::SetCloseUpCam(AActor* TargetActor, FVector ForwardDirction)
{
	SetFocusTarget(TargetActor);
	FVector TargetActorLocation = TargetActor->GetActorLocation();
	FVector TargetLocation = TargetActorLocation + FVector(0, 0, UpwardDistance /2);
	FVector ForwardUnitVec = ForwardDirction.GetSafeNormal2D();
	FVector RightDirection = FVector::CrossProduct(ForwardUnitVec, FVector(0, 0, 1));
	FVector NewPawnPosition = TargetActorLocation + (ForwardUnitVec * 200) + (RightDirection * 200);

	EndLocation = TargetActorLocation + (ForwardUnitVec * 170) + (RightDirection * 70) + FVector(0, 0, UpwardDistance / 3);

	FRotator NewPawnRotation = UKismetMathLibrary::FindLookAtRotation(NewPawnPosition, TargetLocation);

	SetActorLocation(NewPawnPosition);
	SetActorRotation(NewPawnRotation);

	bCameraMoving = true;
}

void APlayerPawnInAiming::SetDeathCam(const FVector AimingCharLoc, AActor* MurderedActor)
{
	SetFocusTarget(MurderedActor);
	FVector MurderedCharLocation = MurderedActor->GetActorLocation();
	FVector StraightLineDirection = (AimingCharLoc - MurderedCharLocation).GetSafeNormal();
	FVector RightDirection = FVector::CrossProduct(StraightLineDirection, FVector(0, 0, 1));
	FVector NewPawnPosition = MurderedCharLocation + (StraightLineDirection * 200) + (RightDirection * RightDistance) + FVector(0, 0, UpwardDistance);

	FRotator NewPawnRotation = UKismetMathLibrary::FindLookAtRotation(NewPawnPosition, MurderedCharLocation);
	SetActorLocation(NewPawnPosition);
	SetActorRotation(NewPawnRotation);

	bCameraMoving = true;
};

void APlayerPawnInAiming::StopCameraMoving() 
{
	bCameraMoving = false;
}

FVector APlayerPawnInAiming::GetActorsHeadLocation() const
{
	return BoneToFocus->GetBoneLocation(FName("head"));
}


void APlayerPawnInAiming::SetFocusTarget(AActor* TargetActor)
{
	USkeletalMeshComponent* ActorsSkeletal = TargetActor->FindComponentByClass<USkeletalMeshComponent>();
	if (ActorsSkeletal)
	{
		BoneToFocus = ActorsSkeletal;
		bFocusHead = true;
	}
	else
	{
		ActorToFocus = TargetActor;
		bFocusHead = false;
	}
}