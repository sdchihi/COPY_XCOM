// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/CustomUserWidget.h"
#include "XCOMPlayerController.h"
#include "CustomThirdPerson.h"
#include "CombatWidget.generated.h"


DECLARE_DYNAMIC_DELEGATE_ThreeParams(FChangeViewTargetDelegate, AActor*, TargetActor, bool, bPlayBlend, int8, InfoIndex);
DECLARE_DYNAMIC_DELEGATE_OneParam(FStartAttackDelegate, const int32, TargetEnemyIndex);
DECLARE_DYNAMIC_DELEGATE(FStartTrajectoryDelegate);
DECLARE_DYNAMIC_DELEGATE(FStartAmbushDelegate);
DECLARE_DYNAMIC_DELEGATE(FStartVisilianceDelegate);

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

	/**
	* Combat Widget을 새로 갱신합니다.
	* @param AimingInfoArray - AimingInfo 배열
	* @param PossibleActionMapWrapper - 가능한 Action
	*/
	UFUNCTION()
	void Renew(const TArray<struct FAimingInfo>& AimingInfoArray, const FPossibleActionWrapper& PossibleActionMapWrapper);
	
	/**
	* 선택된 AimingInfo에 따라 사격 관련 Widget 내용을 갱신합니다.
	* @param AimingInfo - 선택된 AimingInfo
	*/
	void FillAimingInfo(const FAimingInfo& AimingInfo);

	/**
	* 선택된 AimingInfo에 따라 치명타 관련 Widget 내용을 갱신합니다.
	* @param AimingInfo - 선택된 AimingInfo
	*/
	void FillCriticalShotInfo(const FAimingInfo& AimingInfo);

	/**
	* 왼쪽에 위치한 사격 Widget 내용을 갱신합니다.
	* @param Explanation - 사격 요인에 해당하는 설명
	* @param Percentage - 사격 요인 값
	*/
	void FillLeftContnents(const FString& Explanation, const float Percentage);

	/**
	* 왼쪽에 위치한 치명타 Widget 내용을 갱신합니다.
	* @param Explanation - 치명타 요인에 해당하는 설명
	* @param Percentage - 치명타 요인 값
	*/
	void FillRightContents(const FString& Explanation, const float Percentage);

	/** 사격 가능한 적 목록을 표시하는 버튼을 추가합니다. */
	void FillEnemyList();

	/**
	* Enemy 를 표시하는 버튼이 클릭되었을때 호출됩니다.
	* @param ButtonIndex - 선택된 버튼의 인덱스
	*/
	UFUNCTION()
	void EnemyButtonClicked(int32 ButtonIndex);

	UFUNCTION()
	void GrenadeButtonClicked();

	/** 엄폐 버튼 클릭시 호출됩니다. */
	UFUNCTION()
	void VisilianceButtonClicked();

	/** 잠복 버튼 클릭시 호출됩니다. */
	UFUNCTION()
	void AmbushButtonClicked();

	/** 수행가능한 Action의 버튼들을 추가합니다. */
	void FillActionButtonList();

	/** 공격 버튼 클릭시 호출됩니다. */
	UFUNCTION()
	void AttackButtonClicked();

	UFUNCTION()
	void StartAttackButtonClicked();

	UFUNCTION()
	void StartVigilanceButtonClicked();
	
	UFUNCTION()
	void StartAmbushButtonClicked();

	/** 수류탄 투척 버튼 클릭시 호출됩니다. */
	UFUNCTION()
	void StartGrenadeButtonClicked();
};
