// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpringArmComponent.h"
#include "HUDComponent.generated.h"

class UWidgetComponent;
class UUserWidget;
/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class XCOM_API UHUDComponent : public USpringArmComponent
{
	GENERATED_BODY()
	

public:
	UHUDComponent();

	virtual void BeginPlay() override;

	/** ü�� �� ����*/
	UPROPERTY(BlueprintReadOnly)
	UWidgetComponent* HPBarWidget = nullptr;

	void SetWidgetVisibility(const bool bVisiblity);

	void SetVisibilityLocker(bool bLock);

	UUserWidget* GetHPBarWidgetObj();

private:
	bool bLockVisibility = false;

	USpringArmComponent* SpringArm = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> HUDWidgetBlueprint;

	/**
	* �������κ��� ���� �Ÿ��� ������ SpringArm�� ���̸� �����մϴ�.
	* @param Length - ������ SpringArm�� ����
	*/
	UFUNCTION()
	void ControlSpringArmLength(const float Length);

	/**
	* �÷��̾� ī�޶��� ȸ��������  HUD�� ��ġ�� �����մϴ�.
	* @param AxisValue - ���콺 x�� �̵� �Է� ���� �޾ƿɴϴ�.
	*/
	UFUNCTION()
	void HoverHUD(const float AxisValue);
};
