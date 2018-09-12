// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomThirdPerson.h"
#include "GameFramework/PlayerController.h"
#include "XCOMPlayerController.generated.h"

// TMap�� , �� ���� Parser������ Delegate���� ������� ���ϹǷ� ���������� Struct�� �����Ŀ� �����Ѵ�.
USTRUCT()
struct FPossibleActionWrapper
{
	GENERATED_BODY()

	TMap<EAction, bool> PossibleAction;
};

DECLARE_DYNAMIC_DELEGATE_TwoParams(FDeleverInfoDelegate, const TArray<FAimingInfo>&, AiminigInfo, const FPossibleActionWrapper&, PossibleAction);
DECLARE_DYNAMIC_DELEGATE_OneParam(FHealthBarVisiblityDelegate,const bool, bVisible);

class ATileManager;
class APlayerPawnInAiming;
class Path;
class APlayerPawn;

/**
 * 
 */
UCLASS()
class XCOM_API AXCOMPlayerController : public APlayerController
{
	GENERATED_BODY()
	
	
public:
	AXCOMPlayerController();

	virtual void Tick(float DeltaTime) override;

	virtual void BeginPlay() override;

	FDeleverInfoDelegate DeleverInfoDelegate;

	FHealthBarVisiblityDelegate HealthBarVisiblityDelegate;

	UPROPERTY(BlueprintReadOnly)
	APlayerPawn* DefaultPlayerPawn = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<class UCombatWidget> CombatWidgetBlueprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AActiveTileIndicator> TileIndicatorBlueprint;

	// Variable to hold the widget After Creating it.
	UCombatWidget* CombatWidget;

	/** ��Ŀ���� Ȱ��ȭ �մϴ�.
	* @param ActorToFocus - ��Ŀ���� Actor
	* @param bStopAutoMatically - ��Ŀ�� �ڵ� ���� ����
	*/
	void EnableFocusing(AActor* ActorToFocus , bool bStopAutoMatically);

	/** ��Ŀ���� ��Ȱ��ȭ �մϴ�. */
	void DisableFocusing();

	void SetFocusedActor(AActor& abc) { FocusedActor = &abc; };

	/** �÷��̾� ���� �����Ҷ� ȣ��� */
	void FinishPlayerTurn();

	/**
	* �� ���� �� ���� Unit���� ������ ����ϴ�.
	* @param bTeam - ������ ���߰� �� ��
	*/
	UFUNCTION(BlueprintCallable)
	void FocusNextAvailablePlayerUnit(bool bTeam);

	/** Ÿ�� ǥ�ñ⸦ ����ϴ�.	*/
	void HideTileIdicator();

	/** Ÿ�� ǥ�ñ⸦ ��Ÿ�����մϴ�. */
	void ShowTileIdicator();

	void SelectCharacter(ACustomThirdPerson* Unit) { SelectedCharacter = Unit; };

protected:
	virtual void SetupInputComponent() override;

private:
	/** ���� ī�޶� Ư�� ��󿡰� ������ �����ϰ��ִ��� ����  */
	bool bIsFocusing = false;

	/** ī�޶� ��ǥ ������ ������ �� �ڵ����� Focusing�� �ߴ����� ����  */
	bool bStopFocusingAutomatically = false;

	/** ī�޶� ������ ���߰��ִ� Actor */
	UPROPERTY()
	AActor* FocusedActor;

	AActiveTileIndicator* TileIndicator;

	// Character Switching�� �����ϴ� Index
	int32 CharacterSwitchIndex = 1;

	UPROPERTY(VisibleAnywhere)
	ATileManager* TileManager = nullptr;

	/** �÷��̾��� ���ֵ��� �迭 */
	TArray<ACustomThirdPerson*> PlayerCharacters;

	/** ���� ���õ� ���� */
	ACustomThirdPerson* SelectedCharacter= nullptr;

	/** ActionCam �迭 */
	APlayerPawnInAiming* PawnInAimingSituation[2];

	/** �����Ҷ� ����� Crosshair����� ����  */
	class UWidgetComponent* AimWidget = nullptr;

	// ActionCam�� �����ϴ� �÷���
	bool bCameraOrder = false;

	/**
	* �ֱ� ���� Action Cam  Actor�� �����ɴϴ�.
	* @return APlayerPawnInAiming
	*/
	APlayerPawnInAiming* GetCurrentActionCam();

	void Initialize();

	/** ���콺 Ŭ���� �����ϴ� �޼ҵ�*/
	void OnClick();

	/**
	* ���� �� �ȿ��� ���� ���õǾ� �ִ� ĳ���Ͱ� �ƴ� �ٸ� ĳ���ͷ� ��ȯ�ɶ� ȣ��˴ϴ�.
	* @param TargetCharacter - ��ȯ�� ĳ����
	*/
	void SwitchCharacter(ACustomThirdPerson* TargetCharacter);

	/**
	* ĳ���͸� �������� 4�������� �ֺ��� ���� �ִ��� Ȯ���մϴ�.
	* @param MovingCharacter - ����Ȯ���� �� ĳ����
	*/
	void CheckWallAround(ACustomThirdPerson* TargetCharacter);

	/**
	* ĳ���Ͱ� ��ġ�� Ÿ���� ���ö� ȣ���մϴ�.
	* @param TargetCharacter
	* @return �ش� Ÿ���� ������
	*/
	ATile* GetOverlappedTile(ACustomThirdPerson* TargetCharacter);

	/**
	* Cadinal ���⿡ ���ؼ� ���� �ִ��� Ȯ���մϴ�.
	* @param CharacterIndex - ĳ���Ͱ� ��ġ�� Ÿ���� �ε���
	* @param CardinalIndex - Cardinal ������ Ÿ�� �ε���
	*/
	void CheckWallAroundOneDirection(const int32 CharacterIndex, const int CardinalIndex, ACustomThirdPerson* TargetCharacter);

	bool CheckClickedCharacterTeam(ACustomThirdPerson* ClickedCharacter);

	/** �⺻ RTS �������� ���ư��� ȣ���մϴ�.*/
	UFUNCTION()
	void ChangeToDefaultPawn();

	/**
	* ĳ���Ͱ� Ÿ���� �̵��� �� �ֵ��� �˰��� ����� Ÿ���� Visibility�� �����մϴ�.
	* @param OverlappedTile - ���õ� ĳ���Ͱ� �ö��ִ� Ÿ��
	* @param MovingAbility - �̵� ������ ĭ ��
	* @param MovableStepPerAct - 1 Action point�� �̵������� ĭ ��
	*/
	UFUNCTION()
	void SetTilesToUseSelectedChararacter(ATile* OverlappedTile, const int32 MovingAbility, const int32 MovableStepPerAct);

	/**
	* Action Cam �������� �����մϴ�
	* @param StartLocation
	* @param TargetLocation
	* @param ���� ����
	*/
	void ChangeViewTarget(const FVector StartLocation, const FVector TargetLocation, bool bPlayBlend);

	/**
	* Combat Widget�� ���� Action Cam �������� �ε巴�� �̵��մϴ�. (AimWidget�� ǥ�õ˴ϴ�.)
	* @param TagetActor
	* @param bPlayBlend ī�޶��� ���� ����
	* @param InfoIndex ����ϰԵ� ������ AiminigInfo Index
	*/
	UFUNCTION()
	void ChangeViewTargetByCombatWidget(AActor* TargetActor, bool bPlayBlend, int8 InfoIndex);
	
	/**
	* Combat Widget�� ���� Action Cam �������� �ε巴�� �̵��մϴ�.
	* @param TagetActor
	* @param bPlayBlend ī�޶��� ���� ����
	*/
	UFUNCTION()
	void ChangeViewTargetByCombatWidgetWithoutAim(AActor* TargetActor, bool bPlayBlend);


	UFUNCTION()
	void ChangeViewTargetByCharacter(const FVector CharacterLocation, const FVector TargetLocation);

	UFUNCTION()
	void ChangeToDeathCam(AActor* MurderedActor);

	/**
	* ĳ������ ������ ���� Action Cam ���� ������ �ٲߴϴ�.
	* @param TargetActor
	*/
	UFUNCTION()
	void ChangeToFrontCam(AActor* TargetActor);


	UFUNCTION()
	void ChangeToCloseUpCam(AActor* TargetActor, FVector ForwardDirection);

	/**
	* ���� Action Cam  Actor�� �����ɴϴ�.
	* @return APlayerPawnInAiming
	*/
	APlayerPawnInAiming* GetNextActionCam();

	/** Cancel ��ų�� ȣ��˴ϴ�.*/
	void CancelWithESC();

	/**
	* ĳ���Ϳ��� ������ ����մϴ�.
	* @param TargetEnemyIndex - ������ ����� �ε���
	*/
	UFUNCTION()
	void OrderAttack(const int32 TargetEnemyIndex);

	/** ĳ���Ϳ��� ����ź �˵� ������ ����մϴ�.*/
	UFUNCTION()
	void OrderStartTrajectory();

	/** ����ź �˵� ������ ����ϴ�.*/
	void OrderFinishTrajectory();

	/** ��� ����� �����ϴ�.*/
	UFUNCTION()
	void OrderStartVigilance();

	/** Cambat Widget �� ����ϴ�.*/
	UFUNCTION()
	void SetInvisibleCombatWidget();

	/**
	* ĳ���Ͱ� �̵��� ������ ���� Ȯ��, �̵� ���� Ÿ�ϵ��� �����մϴ�.
	* @param MovingCharacter - �̵��ϴ� ĳ����
	*/
	UFUNCTION()
	void AfterCharacterMoving(ACustomThirdPerson* MovingCharacter);

	/**
	* Aiming Widget�� �ۼ����������� �����մϴ�.
	* @param AiminigUnit - ���� ������ �ϰ��ִ� Unit
	* @param AimedUnit - ���� ���ش��ϰ��ִ� Unit
	* @param InfoIndex - Widget���� �����ϰ��ִ� AimingInfo�� Index
	*/
	void SetAiminigWidgetFactor(ACustomThirdPerson* AiminigUnit, ACustomThirdPerson* AimedUnit, int8 InfoIndex);

};
