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
	APlayerPawn();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	/** 카메라의 수평 이동 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float CameraMovementSpeed = 50;

	/** 카메라의 수직 이동 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float CameraLiftingSpeed = 100;

	/** 카메라가 이동하게 되는 경계 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float CameraScrollBoundary =25;

	/** 카메라의 회전 이동 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float CameraHoverSpeed = 5;

	/** 카메라 회전에 맞춰 Unit의 HUD가 회전하게함 */
	FRotateUIDelegate RotateUIDelegate;

	FControlDistanceToUIDelegate ControlDistanceToUIDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float MinHeight = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float MaxHeight = 1500;

	/**
	* 지정한 Actor의 위치로 Lerp 이동합니다.
	* @param TargetActor 지정할 Actor
	*/
	void MoveToTarget(AActor& TargetActor);

	/**
	* 정해진 위치로 카메라를 회전시킵니다.
	* @param PrevArmYaw 이전 Arm Yaw
	* @param LerpAlpha
	*/
	UFUNCTION(BlueprintCallable)
	void TurnCamera(const float PrevArmYaw,const float LerpAlpha);

	/**
	* 다음 카메라의 위치를 받아옵니다..
	* @param bTurnCameraClockWise 정방향 회전 여부
	* @return 다음 카메라의 위치 
	*/
	UFUNCTION(BlueprintCallable)
	float GetNextCameraArmYaw(const bool bTurnCameraClockWise);

private:
	/** 현재 카메라 높이 */
	float CurrentHeight = 0;

	/** 다음 카메라 목표 높이 */
	float NextHeight = 0;

	/** 카메라와 이어져있는 SpringArm */
	float ArmYaw = 0;

	/** Fade in, Fade out 실행 여부 ( == 부드러운 카메라 이동 실행 여부) */
	bool bExecuteZoom = false;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* RootSceneComponent = nullptr;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* SpringArm = nullptr;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera = nullptr;

	/** 확대 기능 작동 여부 */
	bool bCanScroll = false;

	/** 오른쪽 마우스로 카메라 회전 작동 여부 */
	bool bCanHover = false;

	// 카메라의 위치를 결정할 순서값 (0 ~ 3)
	int32 CameraLocationOrder = 0;

	// 카메라를 전진 또는 후진시킵니다.
	UFUNCTION()
	void MoveCameraForward(float Direction);

	// 카메라를 왼쪽 또는 오른쪽으로 이동시킵니다.
	UFUNCTION()
	void MoveCameraRight(float Direction);

	// 카메라를 회전시킵니다.
	UFUNCTION()
	void HoverCamera(float AxisValue);

	// 카메라의 회전 기능을 활성화 시킵니다.
	UFUNCTION()
	void EnableHover();
	
	// 카메라를 회전 기능을 비활성화 시킵니다.
	UFUNCTION()
	void DisableHover();

	/**
	* Camera의 높이를 허용 범위 내에서 더 높게, 낮게 설정합니다.
	* @param Amount 추가또는 감소시킬 양
	*/
	void SetNewHeightValue(float Amount);

	/** 카메라의 고도를 상승시킵니다. */
	UFUNCTION()
	void LiftCamera();

	/** 카메라를 하강시킵니다. */
	UFUNCTION()
	void LowerCamera();
};
