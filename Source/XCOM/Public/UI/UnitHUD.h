// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UnitHUD.generated.h"

/**
 * 
 */
UCLASS()
class XCOM_API UUnitHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UUserWidget> HealthPointBlueprint;

	UFUNCTION(BlueprintCallable)
	void RegisterActor(class ACustomThirdPerson* ActorToSet);

	void InitializeHPBar();

private:
	UPROPERTY()
	ACustomThirdPerson* CharacterRef;

	UPROPERTY()
	UHorizontalBox* HPBarBox;
};
