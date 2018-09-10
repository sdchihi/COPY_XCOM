// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRotateUIDelegate, float, Direction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FControlDistanceToUIDelegate, float, Value);

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
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float CameraMovementSpeed = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float CameraLiftingSpeed = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float CameraScrollBoundary =25;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float CameraHoverSpeed = 5;

	FRotateUIDelegate RotateUIDelegate;

	FControlDistanceToUIDelegate ControlDistanceToUIDelegate;

	UFUNCTION(BlueprintCallable)
	void TurnCamera(const float PrevArmYaw,const float LerpAlpha);

	UFUNCTION(BlueprintCallable)
	float GetNextCameraArmYaw(const bool bTurnCameraClockWise);

	float ArmYaw = 0;

	void MoveToTarget(AActor& TargetActor);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float MaxHeight = 1500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float MinHeight = 200;

	float CurrentHeight = 0;

	float EstimatedHeight = 0;

private:

	bool bExecuteZoom = false;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* RootSceneComponent = nullptr;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* SpringArm = nullptr;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera = nullptr;

	bool bCanScroll = false;

	bool bCanHover = false;

	//0 ~ 3
	int32 CameraLocationOrder = 0;
	// ¸Þ¼Òµå

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

	void SetNewHeightValue(float Amount);

	UFUNCTION()
	void LiftCamera();

	UFUNCTION()
	void LowerCamera();
};
