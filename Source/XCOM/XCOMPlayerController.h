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

class ATileManager2;
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

	UPROPERTY(BlueprintReadOnly)
	APlayerPawn* DefaultPlayerPawn = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<class UCombatWidget> CombatWidgetBlueprint;

	// Variable to hold the widget After Creating it.
	UCombatWidget* CombatWidget;


protected:
	virtual void SetupInputComponent() override;

private:
	//변수
	int32 CharacterSwitchIndex = 0;

	UPROPERTY(VisibleAnywhere)
	ATileManager2* TileManager = nullptr;

	TArray<ACustomThirdPerson*> PlayerCharacters;

	ACustomThirdPerson* SelectedCharacter= nullptr;

	APlayerPawnInAiming* PawnInAimingSituation[2];


	//메소드
	void Initialize();

	void OnClick();

	void SwitchCharacter(ACustomThirdPerson* TargetCharacter);

	void MovingStepByStep(const Path Target, const int32 CurrentIndex);

	void CheckWallAround();

	void CheckWallAroundOneDirection(const int32 CharacterIndex, const int CardinalIndex);

	bool CheckClickedCharacterTeam(ACustomThirdPerson* ClickedCharacter);

	UFUNCTION()
	void ChangeToDefaultPawn();

	UFUNCTION()
	void SetTilesToUseSelectedChararacter(ATile* OverlappedTile, const int32 MovingAbility, const int32 MovableStepPerAct);

	void MoveCharacterBasedOnState(int32 TargetTileIndex);

	void ReleaseCharacter();

	UFUNCTION()
	void ChangeViewTargetWithBlend(const FVector TargetLocation);

	bool bCameraOrder = false;

	void CancelWithESC();

	UFUNCTION()
	void OrderAttack(const int32 TargetEnemyIndex);

	UFUNCTION()
	void OrderStartTrajectory();

	void OrderFinishTrajectory();

};
