// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FloatingWidget.generated.h"

/**
 * 
 */
UCLASS()
class XCOM_API UFloatingWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual	void NativeConstruct() override;

	void HideWidget();

private:
	UFUNCTION()
	void ShowCombatInfo(AActor* DamagedActor, float Damage);

};
