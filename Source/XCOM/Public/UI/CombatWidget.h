// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/CustomUserWidget.h"
#include "XCOMPlayerController.h"
#include "CustomThirdPerson.h"
#include "CombatWidget.generated.h"


DECLARE_DYNAMIC_DELEGATE_TwoParams(FChangeViewTargetDelegate, AActor*, TargetActor, bool, bPlayBlend);
DECLARE_DYNAMIC_DELEGATE_OneParam(FStartAttackDelegate, const int32, TargetEnemyIndex);
DECLARE_DYNAMIC_DELEGATE(FStartTrajectoryDelegate);
DECLARE_DYNAMIC_DELEGATE(FStartAmbushDelegate);
DECLARE_DYNAMIC_DELEGATE(FStartVisilianceDelegate);


//DECLARE_DYNAMIC_DELEGATE_OneParam(FStartActionDelegate, FVector*, TargetLoc);
class UImage;
class UButton;
class UVerticalBox;
class UHorizontalBox;
class UCanvasPanel;
/**
 * 
 */
UCLASS()
class XCOM_API UCombatWidget : public UCustomUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	//virtual	bool Initialize() override;	
	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UUserWidget> SideContentBoxBlueprint;


	UFUNCTION(BlueprintCallable)
	void InitializeInBP();

	FChangeViewTargetDelegate ChangeViewTargetDelegate;

	FStartAttackDelegate StartAttackDelegate;

	FStartTrajectoryDelegate StartTrajectoryDelegate;

	FStartAmbushDelegate StartAmbushDelegate;

	FStartVisilianceDelegate StartVisilianceDelegate;

	void ConstructWidgetMinimum();

	void ConstructWidgetRequiredForAttack();

	void ConstructWidgetNormal();

	void HideAllWidget();
protected:

private:
	class AXCOMPlayerController* PlayerController;

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

	UPROPERTY()
	UCanvasPanel* LeftFrame = nullptr;

	UPROPERTY()
	UCanvasPanel* CenterFrame = nullptr;

	UPROPERTY()
	UCanvasPanel* RightFrame = nullptr;

	UPROPERTY()
	UUserWidget* SumAimingProbBox = nullptr;

	UPROPERTY()
	UUserWidget* SumCriticalProbBox = nullptr;

	UFUNCTION()
	void Renew(const TArray<struct FAimingInfo>& AimingInfoArray, const FPossibleActionWrapper& PossibleActionMapWrapper);
	
	void FillAimingInfo(const FAimingInfo& AimingInfo);

	void FillCriticalShotInfo(const FAimingInfo& AimingInfo);

	void FillLeftContnents(const FString& Explanation, const float Percentage);

	void FillRightContents(const FString& Explanation, const float Percentage);

	void FillEnemyList();

	UFUNCTION()
	void EnemyButtonClicked(int32 ButtonIndex);

	UFUNCTION()
	void GrenadeButtonClicked();

	UFUNCTION()
	void VisilianceButtonClicked();

	UFUNCTION()
	void AmbushButtonClicked();

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
