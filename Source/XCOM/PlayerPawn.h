// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USceneComponent;

UCLASS()
class XCOM_API APlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlayerPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float CameraMovementSpeed = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float CameraZoomSpeed = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float CameraScrollBoundary =25;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float CameraHoverSpeed = 5;

private:

	//변수

	UPROPERTY(VisibleAnywhere)
	USceneComponent* RootSceneComponent = nullptr;


	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* SpringArm = nullptr;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera = nullptr;

	bool bCanScroll = false;

	bool bCanHover = false;
	


	// 메소드

	UFUNCTION()
	void CameraZoomIn();

	UFUNCTION()
	void CameraZoomOut();

	UFUNCTION()
	void MoveCameraForward(float Direction);

	UFUNCTION()
	void MoveCameraRight(float Direction);

	UFUNCTION()
	void HoverCamera(float AxisValue);

	UFUNCTION()
	void EnableHover();
	
	UFUNCTION()
	void DisableHover();
};
