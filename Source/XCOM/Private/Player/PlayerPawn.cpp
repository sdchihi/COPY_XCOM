// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerPawn.h"
#include "Engine.h"
#include "Classes/Camera/CameraComponent.h"
#include "Classes/GameFramework/SpringArmComponent.h"
#include "Classes/Components/SceneComponent.h"
#include "Classes/Components/InputComponent.h"
#include "Classes/Engine/GameViewportClient.h"
#include "Classes/Kismet/KismetMathLibrary.h"
#include "PlayerPawn.h"
#include "CustomThirdPerson.h"


APlayerPawn::APlayerPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(RootSceneComponent);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(RootSceneComponent);
	SpringArm->TargetArmLength = 700.0f;
	SpringArm->bInheritYaw = false;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Follow Camera"));
	FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
}

void APlayerPawn::BeginPlay()
{
	NextHeight = MinHeight;
	Super::BeginPlay();
}

void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector2D mousePosition;
	FVector2D viewportSize;

	//Get mouse position and screen size
	UGameViewportClient* gameViewport = GEngine->GameViewport;
	
	//Make sure viewport exists
	check(gameViewport);
	gameViewport->GetViewportSize(viewportSize);

	if (gameViewport->IsFocused(gameViewport->Viewport) && gameViewport->GetMousePosition(mousePosition))
	{
		//Check if the mouse is at the left or right edge of the screen and move accordingly
		if (mousePosition.X < CameraScrollBoundary)
		{
			MoveCameraRight(-1.0f );
		}
		else if (viewportSize.X - mousePosition.X < CameraScrollBoundary)
		{
			MoveCameraRight(1.0f );
		}

		//Check if the mouse is at the top or bottom edge of the screen and move accordingly
		if (mousePosition.Y < CameraScrollBoundary)
		{
			MoveCameraForward(1.0f );
		}
		else if (viewportSize.Y - mousePosition.Y < CameraScrollBoundary)
		{
			MoveCameraForward(-1.0f);
		}
	}

	if (bExecuteZoom) 
	{
		FVector ActorLocation = GetActorLocation();
		float DeltaHeight = UKismetMathLibrary::Lerp(ActorLocation.Z, NextHeight, DeltaTime);
		ActorLocation.Z = DeltaHeight;
		SetActorLocation(ActorLocation);

		if (FMath::IsNearlyEqual(DeltaHeight, NextHeight, 50.f)) 
		{
			bExecuteZoom = false;
		};
		//	ControlDistanceToUIDelegate.Broadcast((PlayerPawnSpringArmLength - 100) / 4 + 150); 줌될때 사용되던 delegate
	}
}

// Called to bind functionality to input
void APlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	PlayerInputComponent->BindAction("ZoomIn", IE_Pressed, this, &APlayerPawn::LiftCamera);
	PlayerInputComponent->BindAction("ZoomOut", IE_Pressed, this, &APlayerPawn::LowerCamera);

	PlayerInputComponent->BindAction("HoverCharacter", IE_Pressed, this, &APlayerPawn::EnableHover);
	PlayerInputComponent->BindAction("HoverCharacter", IE_Released, this, &APlayerPawn::DisableHover);

	PlayerInputComponent->BindAxis("Hover", this, &APlayerPawn::HoverCamera);
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerPawn::MoveCameraForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerPawn::MoveCameraRight);
}

void APlayerPawn::MoveCameraForward(float Direction) 
{
	float movementValue= Direction* FApp::GetDeltaTime() * CameraMovementSpeed;;
	FVector DeltaMovement = movementValue * FRotator(0.0f, FollowCamera->ComponentToWorld.Rotator().Yaw, 0.0f).Vector();
	FVector newLocation = this->GetActorLocation() + DeltaMovement;

	//Set the new location of the pawn
	SetActorLocation(newLocation);
}

void APlayerPawn::MoveCameraRight(float Direction) 
{
	float movementValue = Direction* FApp::GetDeltaTime() * CameraMovementSpeed;;
	FVector DeltaMovement = movementValue * (FRotator(0.0f, 90.0f, 0.0f) + FRotator(0.0f, FollowCamera->ComponentToWorld.Rotator().Yaw, 0.0f) ).Vector();
	FVector newLocation = this->GetActorLocation() + DeltaMovement;

	//Set the new location of the pawn
	SetActorLocation(newLocation);
}


void APlayerPawn::EnableHover()
{
	bCanHover = true;
	bCanScroll = false;	
}

void APlayerPawn::DisableHover()
{
	bCanHover = false;
	bCanScroll = true;
}


void APlayerPawn::HoverCamera(float AxisValue) 
{
	if (bCanHover) 
	{
		FRotator newRotator = SpringArm->GetComponentRotation() + FRotator(0, AxisValue * CameraHoverSpeed, 0);
		SpringArm->SetWorldRotation(newRotator);
		RotateUIDelegate.Broadcast(AxisValue);
	}
}

void APlayerPawn::TurnCamera(const float PrevArmYaw, const float LerpAlpha)
{
	FRotator SpringArmRotator = SpringArm->GetComponentRotation();
	
	SpringArm->SetWorldRotation(FRotator(SpringArmRotator.Pitch, FMath::Lerp(PrevArmYaw, ArmYaw, LerpAlpha), SpringArmRotator.Roll));
}

float APlayerPawn::GetNextCameraArmYaw(const bool bTurnCameraClockWise)
{
	float PrevArmYaw = ArmYaw;
	if (bTurnCameraClockWise)
	{
		ArmYaw += 90;
	}
	else
	{
		ArmYaw -= 90;
	}
	
	return PrevArmYaw;
}

void APlayerPawn::MoveToTarget(AActor& TargetActor)  
{
	FVector ActorLocation = GetActorLocation();
	FVector TargetLocation = TargetActor.GetActorLocation();
	
	FVector DirectionFromTargetToCamera = (ActorLocation - TargetLocation).GetSafeNormal2D();
	TargetLocation.Z = ActorLocation.Z;

	FVector DeltaMovement = UKismetMathLibrary::VLerp(ActorLocation, TargetLocation, GetWorld()->GetDeltaSeconds() * 5);
	SetActorLocation(DeltaMovement);
}

void APlayerPawn::LiftCamera() 
{
	SetNewHeightValue(CameraLiftingSpeed);
}

void APlayerPawn::LowerCamera()
{
	SetNewHeightValue(-CameraLiftingSpeed);
}

void APlayerPawn::SetNewHeightValue(float Amount) 
{
	float EstimatedHeightBeforeChecking = NextHeight + Amount;
	if (EstimatedHeightBeforeChecking < MaxHeight && MinHeight < EstimatedHeightBeforeChecking)
	{
		NextHeight += Amount;
		bExecuteZoom = true;
	}	
}

