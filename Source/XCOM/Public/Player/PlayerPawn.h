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
	
	/** ī�޶��� ���� �̵� �ӵ� */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float CameraMovementSpeed = 50;

	/** ī�޶��� ���� �̵� �ӵ� */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float CameraLiftingSpeed = 100;

	/** ī�޶� �̵��ϰ� �Ǵ� ��� */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float CameraScrollBoundary =25;

	/** ī�޶��� ȸ�� �̵� �ӵ� */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float CameraHoverSpeed = 5;

	/** ī�޶� ȸ���� ���� Unit�� HUD�� ȸ���ϰ��� */
	FRotateUIDelegate RotateUIDelegate;

	FControlDistanceToUIDelegate ControlDistanceToUIDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float MinHeight = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float MaxHeight = 1500;

	/**
	* ������ Actor�� ��ġ�� Lerp �̵��մϴ�.
	* @param TargetActor ������ Actor
	*/
	void MoveToTarget(AActor& TargetActor);

	/**
	* ������ ��ġ�� ī�޶� ȸ����ŵ�ϴ�.
	* @param PrevArmYaw ���� Arm Yaw
	* @param LerpAlpha
	*/
	UFUNCTION(BlueprintCallable)
	void TurnCamera(const float PrevArmYaw,const float LerpAlpha);

	/**
	* ���� ī�޶��� ��ġ�� �޾ƿɴϴ�..
	* @param bTurnCameraClockWise ������ ȸ�� ����
	* @return ���� ī�޶��� ��ġ 
	*/
	UFUNCTION(BlueprintCallable)
	float GetNextCameraArmYaw(const bool bTurnCameraClockWise);

private:
	/** ���� ī�޶� ���� */
	float CurrentHeight = 0;

	/** ���� ī�޶� ��ǥ ���� */
	float NextHeight = 0;

	/** ī�޶�� �̾����ִ� SpringArm */
	float ArmYaw = 0;

	/** Fade in, Fade out ���� ���� ( == �ε巯�� ī�޶� �̵� ���� ����) */
	bool bExecuteZoom = false;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* RootSceneComponent = nullptr;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* SpringArm = nullptr;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera = nullptr;

	/** Ȯ�� ��� �۵� ���� */
	bool bCanScroll = false;

	/** ������ ���콺�� ī�޶� ȸ�� �۵� ���� */
	bool bCanHover = false;

	// ī�޶��� ��ġ�� ������ ������ (0 ~ 3)
	int32 CameraLocationOrder = 0;

	// ī�޶� ���� �Ǵ� ������ŵ�ϴ�.
	UFUNCTION()
	void MoveCameraForward(float Direction);

	// ī�޶� ���� �Ǵ� ���������� �̵���ŵ�ϴ�.
	UFUNCTION()
	void MoveCameraRight(float Direction);

	// ī�޶� ȸ����ŵ�ϴ�.
	UFUNCTION()
	void HoverCamera(float AxisValue);

	// ī�޶��� ȸ�� ����� Ȱ��ȭ ��ŵ�ϴ�.
	UFUNCTION()
	void EnableHover();
	
	// ī�޶� ȸ�� ����� ��Ȱ��ȭ ��ŵ�ϴ�.
	UFUNCTION()
	void DisableHover();

	/**
	* Camera�� ���̸� ��� ���� ������ �� ����, ���� �����մϴ�.
	* @param Amount �߰��Ǵ� ���ҽ�ų ��
	*/
	void SetNewHeightValue(float Amount);

	/** ī�޶��� ���� ��½�ŵ�ϴ�. */
	UFUNCTION()
	void LiftCamera();

	/** ī�޶� �ϰ���ŵ�ϴ�. */
	UFUNCTION()
	void LowerCamera();
};
