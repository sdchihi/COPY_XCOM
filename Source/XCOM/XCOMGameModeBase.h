// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "XCOMGameModeBase.generated.h"

class ACustomThirdPerson;
/**
 * 
 */
UCLASS()
class XCOM_API AXCOMGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AXCOMGameModeBase();
	
	virtual void BeginPlay() override;

	void CheckTurnOver(const bool bIsPlayerTeam);

	void CheckTurnStateOfOneTeam(TArray<ACustomThirdPerson>& Characters);


private:
	TArray<ACustomThirdPerson> PlayerCharacters;

	TArray<ACustomThirdPerson> EnemyCharacters;
	

};
