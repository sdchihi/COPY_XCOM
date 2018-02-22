// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerPawn.h"
#include "Engine.h"
#include "Classes/Camera/CameraComponent.h"
#include "Classes/GameFramework/SpringArmComponent.h"
#include "Classes/Components/SceneComponent.h"
#include "Classes/Components/InputComponent.h"
#include "Classes/Engine/GameViewportClient.h"

// Sets default values
APlayerPawn::APlayerPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(RootSceneComponent);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(RootSceneComponent);
	SpringArm->TargetArmLength = 700.0f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Follow Camera"));
	FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

}

// Called when the game starts or when spawned
void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
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
			MoveCameraRight(-1.0f * DeltaTime);
		}
		else if (viewportSize.X - mousePosition.X < CameraScrollBoundary)
		{
			MoveCameraRight(1.0f * DeltaTime);
		}

		//Check if the mouse is at the top or bottom edge of the screen and move accordingly
		if (mousePosition.Y < CameraScrollBoundary)
		{
			MoveCameraForward(1.0f * DeltaTime);
		}
		else if (viewportSize.Y - mousePosition.Y < CameraScrollBoundary)
		{
			MoveCameraForward(-1.0f * DeltaTime);
		}
	}

}

// Called to bind functionality to input
void APlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	PlayerInputComponent->BindAction("ZoomIn", IE_Pressed, this, &APlayerPawn::CameraZoomIn);
	PlayerInputComponent->BindAction("ZoomOut", IE_Pressed, this, &APlayerPawn::CameraZoomOut);

	PlayerInputComponent->BindAction("HoverCharacter", IE_Pressed, this, &APlayerPawn::EnableHover);
	PlayerInputComponent->BindAction("HoverCharacter", IE_Released, this, &APlayerPawn::DisableHover);

	PlayerInputComponent->BindAxis("Hover", this, &APlayerPawn::HoverCamera);
}



void APlayerPawn::CameraZoomIn() 
{
	SpringArm->TargetArmLength = FMath::Clamp(SpringArm->TargetArmLength - CameraZoomSpeed, 100.f, 700.f);
}

void APlayerPawn::CameraZoomOut() 
{
	SpringArm->TargetArmLength = FMath::Clamp(SpringArm->TargetArmLength + CameraZoomSpeed, 100.f, 700.f);
}

void APlayerPawn::MoveCameraForward(float Direction) 
{
	if (!bCanScroll) { return; }

	float movementValue = Direction * CameraMovementSpeed;

	//Create a delta vector that moves by the movementValue in the direction of the camera's yaw
	FVector DeltaMovement = movementValue * FRotator(0.0f, FollowCamera->ComponentToWorld.Rotator().Yaw, 0.0f).Vector();

	//Add the delta to a new vector   카메라의 Up Vector 의 Unit Vector
	FVector newLocation = this->GetActorLocation() + DeltaMovement;

	//Set the new location of the pawn
	SetActorLocation(newLocation);
}

void APlayerPawn::MoveCameraRight(float Direction) 
{
	if (!bCanScroll) { return; }


	float movementValue = Direction * CameraMovementSpeed;

	//Create a delta vector that moves by the movementValue in the direction of the right of the camera's yaw
	FVector DeltaMovement = movementValue * (FRotator(0.0f, 90.0f, 0.0f) + FRotator(0.0f, FollowCamera->ComponentToWorld.Rotator().Yaw, 0.0f) ).Vector();

	//Add the delta to a new vector
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
	}
}