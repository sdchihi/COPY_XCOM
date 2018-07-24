// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/CustomUserWidget.h"
#include "UnitHUD.generated.h"

/**
 * 
 */
UCLASS()
class XCOM_API UUnitHUD : public UCustomUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void RegisterActor(class ACustomThirdPerson* ActorToSet);

	void InitializeHPBar();

	void ReduceHP(int8 Damage);

private:
	UPROPERTY()
	ACustomThirdPerson* CharacterRef;

	UPROPERTY()
	class UUniformGridPanel* GridPannel;

	UPROPERTY()
	class UImage* TeamImage;

	TArray <UCustomUserWidget* > HPWidgetArray;

	void SetTeamIconImage();

	UClass* GetHPClass(bool bIsPlayerTeam);

};
