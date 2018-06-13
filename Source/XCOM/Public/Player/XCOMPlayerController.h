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

	UFUNCTION(BlueprintCallable)
	FVector GetNextAvailableCharLocation();

	FDeleverInfoDelegate DeleverInfoDelegate;

	FHealthBarVisiblityDelegate HealthBarVisiblityDelegate;

	UPROPERTY(BlueprintReadOnly)
	APlayerPawn* DefaultPlayerPawn = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<class UCombatWidget> CombatWidgetBlueprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class ACoveringChecker> CoveringCheckerBlueprint;

	// Variable to hold the widget After Creating it.
	UCombatWidget* CombatWidget;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SwitchNextCharacter(bool bTeam);

protected:
	virtual void SetupInputComponent() override;

private:
	//변수
	int32 CharacterSwitchIndex = 1;

	UPROPERTY(VisibleAnywhere)
	ATileManager* TileManager = nullptr;

	TArray<ACustomThirdPerson*> PlayerCharacters;

	ACustomThirdPerson* SelectedCharacter= nullptr;

	ACoveringChecker* CoveringChecker = nullptr;

	APlayerPawnInAiming* PawnInAimingSituation[2];

	bool bCameraOrder = false;

	//메소드
	void Initialize();

	void OnClick();

	void SwitchCharacter(ACustomThirdPerson* TargetCharacter);

	void CheckWallAround(ACustomThirdPerson* TargetCharacter);

	ATile* GetOverlappedTile(ACustomThirdPerson* TargetCharacter);

	void CheckWallAroundOneDirection(const int32 CharacterIndex, const int CardinalIndex, ACustomThirdPerson* TargetCharacter);

	bool CheckClickedCharacterTeam(ACustomThirdPerson* ClickedCharacter);

	UFUNCTION()
	void ChangeToDefaultPawn();

	UFUNCTION()
	void SetTilesToUseSelectedChararacter(ATile* OverlappedTile, const int32 MovingAbility, const int32 MovableStepPerAct);


	void ReleaseCharacter();

	void ChangeViewTargetWithBlend(const FVector StartLocation, const FVector TargetLocation);

	UFUNCTION()
	void ChangeViewTargetByCombatWidget(const FVector TargetLocation);

	UFUNCTION()
	void ChangeViewTargetByCharacter(const FVector CharacterLocation, const FVector TargetLocation);


	UFUNCTION()
	void ChangeToDeathCam(const FVector TargetLocation);

	APlayerPawnInAiming* GetNextActionCam();


	void CancelWithESC();

	UFUNCTION()
	void OrderAttack(const int32 TargetEnemyIndex);

	UFUNCTION()
	void OrderStartTrajectory();

	void OrderFinishTrajectory();

	UFUNCTION()
	void OrderStartVigilance();

	UFUNCTION()
	void SetInvisibleCombatWidget();

	UFUNCTION()
	void AfterCharacterMoving(ACustomThirdPerson* MovingCharacter);

};
