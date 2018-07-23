// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class XCOM_API UCustomUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	float PlayAnimationByName(FName AnimationName);

protected:
	TMap<FName, class UWidgetAnimation*> AnimationsMap;

	UWidgetAnimation* GetAnimationByName(FName AnimationName) const;

	void FillAnimationsMap();

private:
	
	
};
