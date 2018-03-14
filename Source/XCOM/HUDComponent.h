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

	UPROPERTY(BlueprintReadOnly)
	UWidgetComponent* HPBarWidget = nullptr;;

private:

	USpringArmComponent* SpringArm = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> HUDWidgetBlueprint;

	UFUNCTION()
	void ControlSpringArmLength(float value);

	UFUNCTION()
	void HoverHUD(float AxisValue);
};
