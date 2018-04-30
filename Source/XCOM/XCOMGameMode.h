// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "XCOMGameMode.generated.h"

class ACustomThirdPerson;
class AFogOfWarManager;
/**
 * 
 */
UCLASS()
class XCOM_API AXCOMGameMode : public AGameMode
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;


public:
	AXCOMGameMode();
	

	UFUNCTION()
	void CheckTurnOver(const bool bIsPlayerTeam);

	void CheckTurnStateOfOneTeam(TArray<ACustomThirdPerson*>& Characters);

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFogOfWarManager> FogOfWarBP;

	TArray<ACustomThirdPerson*> GetTeamMemeber(const bool bTeam);

private:
	TArray<ACustomThirdPerson*> PlayerCharacters;

	TArray<ACustomThirdPerson*> EnemyCharacters;
	
	void RestoreTeamActionPoint(TArray<ACustomThirdPerson*>& Characters);

	UFUNCTION()
	void SetVisibleAllHealthBar(const bool bVisible);

	void SpawnFogOfWar();

	
};
