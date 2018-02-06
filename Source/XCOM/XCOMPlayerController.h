// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "XCOMPlayerController.generated.h"

class ACustomThirdPerson;
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
	void OnClick();

	void SwitchCharacter(ACustomThirdPerson* TargetCharacter);

};
