// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "XCOMPlayerController.generated.h"

class ATileManager2;
class ACustomThirdPerson;
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



protected:
	virtual void SetupInputComponent() override;


private:
	//변수


	UPROPERTY(VisibleAnywhere)
	ATileManager2* TileManager = nullptr;

	ACustomThirdPerson* SelectedCharacter= nullptr;

	APlayerPawnInAiming* PawnInAimingSituation= nullptr;

	APlayerPawn* DefaultPlayerPawn = nullptr;

	//메소드

	void OnClick();

	void SwitchCharacter(ACustomThirdPerson* TargetCharacter);

	void MovingStepByStep(Path Target, int32 CurrentIndex);

	void CheckWallAround();

	void CheckWallAroundOneDirection(int32 CharacterIndex, int CardinalIndex);

	bool CheckClickedCharacterTeam(ACustomThirdPerson* ClickedCharacter);

	UFUNCTION()
	void ChangeToDefaultPawn();

};
