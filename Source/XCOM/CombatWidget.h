// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CombatWidget.generated.h"


class UVerticalBox;
class UHorizontalBox;
/**
 * 
 */
UCLASS()
class XCOM_API UCombatWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	//virtual	bool Initialize() override;	
	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UUserWidget> SideContentBoxBlueprint;

	UFUNCTION(BlueprintCallable)
	void InitializeInBP();


protected:

private:
	void ClearContents(const bool bClearAll = false);

	TArray<struct FAimingInfo> SelectedCharacterAimingInfo;

	UPROPERTY()
	UVerticalBox* LeftVBox = nullptr;;

	UPROPERTY()
	UVerticalBox* RightVBox = nullptr;

	UPROPERTY()
	UHorizontalBox* CenterActionHBox = nullptr;;

	UPROPERTY()
	UHorizontalBox* EnemyIconHBox = nullptr;;

	UFUNCTION()
	void Renew(const TArray<struct FAimingInfo>& AimingInfoArray);
	
	void ConvertToSuitableFormat(const FAimingInfo& AimingInfo, OUT FString& Explanation, OUT float& Percentage);

	void FillContnents(const FString& Explanation, const float Percentage);

	void FillEnemyList();

	UFUNCTION()
	void TempFunction();

	UFUNCTION()
	void EnemyButtonClicked(int32 ButtonIndex);

};
