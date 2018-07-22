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

	void ShowCombatInfo(float Damage, FloatingWidgetState State);

private:
	UPROPERTY()
	class UImage* BackgroundImage;

	void ChangePopUpFactor(FloatingWidgetState State, int8 Damage = 0);

	UPROPERTY()
	UImage* IconImage;

	UPROPERTY()
	class UTextBlock* FigureText;

	UPROPERTY()
	UTextBlock* ExplanationText;


	UFUNCTION()
	void PopDown();

	UFUNCTION()
	void HideWidget();
};
