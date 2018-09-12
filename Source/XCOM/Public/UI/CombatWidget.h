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
	* Combat Widget�� ���� �����մϴ�.
	* @param AimingInfoArray - AimingInfo �迭
	* @param PossibleActionMapWrapper - ������ Action
	*/
	UFUNCTION()
	void Renew(const TArray<struct FAimingInfo>& AimingInfoArray, const FPossibleActionWrapper& PossibleActionMapWrapper);
	
	/**
	* ���õ� AimingInfo�� ���� ��� ���� Widget ������ �����մϴ�.
	* @param AimingInfo - ���õ� AimingInfo
	*/
	void FillAimingInfo(const FAimingInfo& AimingInfo);

	/**
	* ���õ� AimingInfo�� ���� ġ��Ÿ ���� Widget ������ �����մϴ�.
	* @param AimingInfo - ���õ� AimingInfo
	*/
	void FillCriticalShotInfo(const FAimingInfo& AimingInfo);

	/**
	* ���ʿ� ��ġ�� ��� Widget ������ �����մϴ�.
	* @param Explanation - ��� ���ο� �ش��ϴ� ����
	* @param Percentage - ��� ���� ��
	*/
	void FillLeftContnents(const FString& Explanation, const float Percentage);

	/**
	* ���ʿ� ��ġ�� ġ��Ÿ Widget ������ �����մϴ�.
	* @param Explanation - ġ��Ÿ ���ο� �ش��ϴ� ����
	* @param Percentage - ġ��Ÿ ���� ��
	*/
	void FillRightContents(const FString& Explanation, const float Percentage);

	/** ��� ������ �� ����� ǥ���ϴ� ��ư�� �߰��մϴ�. */
	void FillEnemyList();

	/**
	* Enemy �� ǥ���ϴ� ��ư�� Ŭ���Ǿ����� ȣ��˴ϴ�.
	* @param ButtonIndex - ���õ� ��ư�� �ε���
	*/
	UFUNCTION()
	void EnemyButtonClicked(int32 ButtonIndex);

	UFUNCTION()
	void GrenadeButtonClicked();

	/** ���� ��ư Ŭ���� ȣ��˴ϴ�. */
	UFUNCTION()
	void VisilianceButtonClicked();

	/** �ẹ ��ư Ŭ���� ȣ��˴ϴ�. */
	UFUNCTION()
	void AmbushButtonClicked();

	/** ���డ���� Action�� ��ư���� �߰��մϴ�. */
	void FillActionButtonList();

	/** ���� ��ư Ŭ���� ȣ��˴ϴ�. */
	UFUNCTION()
	void AttackButtonClicked();

	UFUNCTION()
	void StartAttackButtonClicked();

	UFUNCTION()
	void StartVigilanceButtonClicked();
	
	UFUNCTION()
	void StartAmbushButtonClicked();

	/** ����ź ��ô ��ư Ŭ���� ȣ��˴ϴ�. */
	UFUNCTION()
	void StartGrenadeButtonClicked();
};
