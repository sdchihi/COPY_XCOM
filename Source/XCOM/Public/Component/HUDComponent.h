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

	/** 체력 바 위젯*/
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
	* 유닛으로부터 일정 거리를 유지할 SpringArm의 길이를 변경합니다.
	* @param Length - 설정할 SpringArm의 길이
	*/
	UFUNCTION()
	void ControlSpringArmLength(const float Length);

	/**
	* 플레이어 카메라의 회전에따라  HUD의 위치를 변경합니다.
	* @param AxisValue - 마우스 x축 이동 입력 값을 받아옵니다.
	*/
	UFUNCTION()
	void HoverHUD(const float AxisValue);
};
