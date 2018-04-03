// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "CustomButton.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FConveyIndexDelegate, int32, BtIndex);

/**
 * 
 */
UCLASS()
class XCOM_API UCustomButton : public UButton
{
	GENERATED_BODY()
	
public:
	UCustomButton();

	void SetButtonIndex(int32 ButtonIndex) { this->ButtonIndex = ButtonIndex; };

	int32 GetButtonIndex() { return ButtonIndex; };

	FConveyIndexDelegate ConveyIndexDelegate;

private:
	int32 ButtonIndex = 0;
	
	UFUNCTION()
	void ConveyButtonIndex();

	
};
