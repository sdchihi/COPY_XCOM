// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerPawnInAiming.generated.h"

class UCameraComponent;

UCLASS()
class XCOM_API APlayerPawnInAiming : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlayerPawnInAiming();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* Camera;

	void SetCameraPositionInAimingSituation(const FVector AimingCharLoc, const FVector AimedCharLoc);

	void SetCameraPositionInDying(const FVector AimingCharLoc, const FVector DyingCharacterLocation);

	UPROPERTY(EditDefaultsOnly)
	float BackWardDistance = 100;

	UPROPERTY(EditDefaultsOnly)
	float RightDistance = 100;

	UPROPERTY(EditDefaultsOnly)
	float UpwardDistance = 100;
};
