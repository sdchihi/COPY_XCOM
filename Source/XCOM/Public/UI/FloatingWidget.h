// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/CustomUserWidget.h"
#include "FloatingWidget.generated.h"

UENUM(BlueprintType)
enum class FloatingWidgetState : uint8
{
	Damaged,
	Dodge,
	Visilance,
	Critical,
	None,
};


/**
 * 
 */
UCLASS()
class XCOM_API UFloatingWidget : public UCustomUserWidget
{
	GENERATED_BODY()
public:
	virtual	void NativeConstruct() override;

	/**
	* 전투 결과를 팝업시킵니다.
	* @param Damage - 데미지 값
	* @param State - 전투 정보
	*/
	void ShowCombatInfo(float Damage, FloatingWidgetState State);

private:
	UPROPERTY()
	class UImage* BackgroundImage;

	UPROPERTY()
	UImage* IconImage;

	UPROPERTY()
	class UTextBlock* FigureText;

	UPROPERTY()
	UTextBlock* ExplanationText;

	/**
	* Widget의 이미지, 텍스트를 변경합니다.
	* @param State - 전투 정보
	* @param Damage - 데미지 값
	*/
	void ChangePopUpFactor(FloatingWidgetState State, int8 Damage = 0);

	/**
	* 팝업된 전투 결과를 천천히 사라지게 합니다.
	*/
	UFUNCTION()
	void PopDown();

	/** 전투 결과를 게임에서 숨깁니다. */
	UFUNCTION()
	void HideWidget();
};
