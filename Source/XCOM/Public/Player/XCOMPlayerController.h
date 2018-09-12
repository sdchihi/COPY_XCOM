// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomThirdPerson.h"
#include "GameFramework/PlayerController.h"
#include "XCOMPlayerController.generated.h"

// TMap은 , 에 대한 Parser문제로 Delegate선언에 사용하지 못하므로 불편하지만 Struct로 감싼후에 선언한다.
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

	/** 포커싱을 활성화 합니다.
	* @param ActorToFocus - 포커싱할 Actor
	* @param bStopAutoMatically - 포커싱 자동 종료 여부
	*/
	void EnableFocusing(AActor* ActorToFocus , bool bStopAutoMatically);

	/** 포커싱을 비활성화 합니다. */
	void DisableFocusing();

	void SetFocusedActor(AActor& abc) { FocusedActor = &abc; };

	/** 플레이어 턴을 종료할때 호출됨 */
	void FinishPlayerTurn();

	/**
	* 턴 종료 후 다음 Unit으로 초점을 맞춥니다.
	* @param bTeam - 초점을 맞추게 될 팀
	*/
	UFUNCTION(BlueprintCallable)
	void FocusNextAvailablePlayerUnit(bool bTeam);

	/** 타일 표시기를 숨깁니다.	*/
	void HideTileIdicator();

	/** 타일 표시기를 나타나게합니다. */
	void ShowTileIdicator();

	void SelectCharacter(ACustomThirdPerson* Unit) { SelectedCharacter = Unit; };

protected:
	virtual void SetupInputComponent() override;

private:
	/** 현재 카메라가 특정 대상에게 초점을 고정하고있는지 여부  */
	bool bIsFocusing = false;

	/** 카메라가 목표 지점에 도달한 후 자동으로 Focusing을 중단할지 여부  */
	bool bStopFocusingAutomatically = false;

	/** 카메라가 초점을 맞추고있는 Actor */
	UPROPERTY()
	AActor* FocusedActor;

	AActiveTileIndicator* TileIndicator;

	// Character Switching을 제어하는 Index
	int32 CharacterSwitchIndex = 1;

	UPROPERTY(VisibleAnywhere)
	ATileManager* TileManager = nullptr;

	/** 플레이어측 유닛들의 배열 */
	TArray<ACustomThirdPerson*> PlayerCharacters;

	/** 현재 선택된 유닛 */
	ACustomThirdPerson* SelectedCharacter= nullptr;

	/** ActionCam 배열 */
	APlayerPawnInAiming* PawnInAimingSituation[2];

	/** 조준할때 생기는 Crosshair모양의 위젯  */
	class UWidgetComponent* AimWidget = nullptr;

	// ActionCam을 제어하는 플래그
	bool bCameraOrder = false;

	/**
	* 최근 사용된 Action Cam  Actor를 가져옵니다.
	* @return APlayerPawnInAiming
	*/
	APlayerPawnInAiming* GetCurrentActionCam();

	void Initialize();

	/** 마우스 클릭에 반응하는 메소드*/
	void OnClick();

	/**
	* 같은 편 안에서 현재 선택되어 있는 캐릭터가 아닌 다른 캐릭터로 전환될때 호출됩니다.
	* @param TargetCharacter - 전환할 캐릭터
	*/
	void SwitchCharacter(ACustomThirdPerson* TargetCharacter);

	/**
	* 캐릭터를 기준으로 4방향으로 주변에 벽이 있는지 확인합니다.
	* @param MovingCharacter - 엄폐확인을 할 캐릭터
	*/
	void CheckWallAround(ACustomThirdPerson* TargetCharacter);

	/**
	* 캐릭터가 위치한 타일을 얻어올때 호출합니다.
	* @param TargetCharacter
	* @return 해당 타일의 포인터
	*/
	ATile* GetOverlappedTile(ACustomThirdPerson* TargetCharacter);

	/**
	* Cadinal 방향에 대해서 벽이 있는지 확인합니다.
	* @param CharacterIndex - 캐릭터가 위치한 타일의 인덱스
	* @param CardinalIndex - Cardinal 방향의 타일 인덱스
	*/
	void CheckWallAroundOneDirection(const int32 CharacterIndex, const int CardinalIndex, ACustomThirdPerson* TargetCharacter);

	bool CheckClickedCharacterTeam(ACustomThirdPerson* ClickedCharacter);

	/** 기본 RTS 시점으로 돌아갈때 호출합니다.*/
	UFUNCTION()
	void ChangeToDefaultPawn();

	/**
	* 캐릭터가 타일을 이동할 수 있도록 알고리즘 수행과 타일의 Visibility를 세팅합니다.
	* @param OverlappedTile - 선택된 캐릭터가 올라가있는 타일
	* @param MovingAbility - 이동 가능한 칸 수
	* @param MovableStepPerAct - 1 Action point로 이동가능한 칸 수
	*/
	UFUNCTION()
	void SetTilesToUseSelectedChararacter(ATile* OverlappedTile, const int32 MovingAbility, const int32 MovableStepPerAct);

	/**
	* Action Cam 시점으로 변경합니다
	* @param StartLocation
	* @param TargetLocation
	* @param 블렌드 여부
	*/
	void ChangeViewTarget(const FVector StartLocation, const FVector TargetLocation, bool bPlayBlend);

	/**
	* Combat Widget에 의해 Action Cam 시점으로 부드럽게 이동합니다. (AimWidget이 표시됩니다.)
	* @param TagetActor
	* @param bPlayBlend 카메라의 블렌드 여부
	* @param InfoIndex 사용하게될 유닛의 AiminigInfo Index
	*/
	UFUNCTION()
	void ChangeViewTargetByCombatWidget(AActor* TargetActor, bool bPlayBlend, int8 InfoIndex);
	
	/**
	* Combat Widget에 의해 Action Cam 시점으로 부드럽게 이동합니다.
	* @param TagetActor
	* @param bPlayBlend 카메라의 블렌드 여부
	*/
	UFUNCTION()
	void ChangeViewTargetByCombatWidgetWithoutAim(AActor* TargetActor, bool bPlayBlend);


	UFUNCTION()
	void ChangeViewTargetByCharacter(const FVector CharacterLocation, const FVector TargetLocation);

	UFUNCTION()
	void ChangeToDeathCam(AActor* MurderedActor);

	/**
	* 캐릭터의 정면을 비출 Action Cam 으로 시점을 바꿉니다.
	* @param TargetActor
	*/
	UFUNCTION()
	void ChangeToFrontCam(AActor* TargetActor);


	UFUNCTION()
	void ChangeToCloseUpCam(AActor* TargetActor, FVector ForwardDirection);

	/**
	* 사용될 Action Cam  Actor를 가져옵니다.
	* @return APlayerPawnInAiming
	*/
	APlayerPawnInAiming* GetNextActionCam();

	/** Cancel 시킬때 호출됩니다.*/
	void CancelWithESC();

	/**
	* 캐릭터에게 공격을 명령합니다.
	* @param TargetEnemyIndex - 공격할 대상의 인덱스
	*/
	UFUNCTION()
	void OrderAttack(const int32 TargetEnemyIndex);

	/** 캐릭터에게 수류탄 궤도 추적을 명령합니다.*/
	UFUNCTION()
	void OrderStartTrajectory();

	/** 수류탄 궤도 추적을 멈춥니다.*/
	void OrderFinishTrajectory();

	/** 경계 명령을 내립니다.*/
	UFUNCTION()
	void OrderStartVigilance();

	/** Cambat Widget 을 감춥니다.*/
	UFUNCTION()
	void SetInvisibleCombatWidget();

	/**
	* 캐릭터가 이동을 끝낸후 엄폐 확인, 이동 가능 타일등을 갱신합니다.
	* @param MovingCharacter - 이동하는 캐릭터
	*/
	UFUNCTION()
	void AfterCharacterMoving(ACustomThirdPerson* MovingCharacter);

	/**
	* Aiming Widget에 퍼센테이지값을 변경합니다.
	* @param AiminigUnit - 현재 조준을 하고있는 Unit
	* @param AimedUnit - 현재 조준당하고있는 Unit
	* @param InfoIndex - Widget에서 지정하고있는 AimingInfo의 Index
	*/
	void SetAiminigWidgetFactor(ACustomThirdPerson* AiminigUnit, ACustomThirdPerson* AimedUnit, int8 InfoIndex);

};
