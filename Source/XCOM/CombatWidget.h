// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "XCOMPlayerController.h"
#include "CustomThirdPerson.h"
#include "CombatWidget.generated.h"


DECLARE_DYNAMIC_DELEGATE_OneParam(FChangeViewTargetDelegate, const FVector, TargetLoc);
DECLARE_DYNAMIC_DELEGATE_OneParam(FStartAttackDelegate, const int32, TargetEnemyIndex);

//DECLARE_DYNAMIC_DELEGATE_OneParam(FStartActionDelegate, FVector*, TargetLoc);
class UButton;
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


	FChangeViewTargetDelegate ChangeViewTargetDelegate;

	FStartAttackDelegate StartAttackDelegate;
//	FStartActionDelegate StartActionDelegate;

protected:

private:
	int32 TargetEnemyIndex = 0;

	void ClearContents(const bool bClearAll = false);

	TArray<struct FAimingInfo> SelectedCharacterAimingInfo;

	TMap<EAction, bool> PossibleActionMap;

	UPROPERTY()
	UVerticalBox* LeftVBox = nullptr;;

	UPROPERTY()
	UVerticalBox* RightVBox = nullptr;

	UPROPERTY()
	UHorizontalBox* CenterActionHBox = nullptr;;

	UPROPERTY()
	UHorizontalBox* EnemyIconHBox = nullptr;;

	UPROPERTY()
	UButton* MainStartActionButton = nullptr;

	UFUNCTION()
	void Renew(const TArray<struct FAimingInfo>& AimingInfoArray, const FPossibleActionWrapper& PossibleActionMapWrapper);
	
	void ConvertToSuitableFormat(const FAimingInfo& AimingInfo, OUT FString& Explanation, OUT float& Percentage);

	void FillContnents(const FString& Explanation, const float Percentage);

	void FillEnemyList();

	UFUNCTION()
	void EnemyButtonClicked(int32 ButtonIndex);

	void FillActionButtonList();

	UFUNCTION()
	void AttackButtonClicked();

	UFUNCTION()
	void StartAttackButtonClicked();

	UFUNCTION()
	void StartVigilanceButtonClicked();
	
	UFUNCTION()
	void StartAmbushButtonClicked();

	UFUNCTION()
	void StartGrenadeButtonClicked();


};
