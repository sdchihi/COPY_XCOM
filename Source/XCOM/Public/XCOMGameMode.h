// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "XCOMGameMode.generated.h"

enum class EDirection : uint8;

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

	UPROPERTY(EditInstanceOnly)
	TArray<AActor*> PlayerUnitDetectorArray;

	UPROPERTY(EditDefaultsOnly, meta = (ToolTip = "Fog of war"))
	TSubclassOf<AFogOfWarManager> FogOfWarBP;

	UPROPERTY(EditDefaultsOnly, meta = (ToolTip = "Fog of war"))
	bool bGenerateFogOfWar = true;

	TArray<ACustomThirdPerson*> GetTeamMemeber(const bool bTeam);

private:
	TArray<ACustomThirdPerson*> PlayerCharacters;

	TArray<ACustomThirdPerson*> EnemyCharacters;

	TMap<int8, TArray<class AEnemyUnit*>> EnemyTeamMap;
	
	void RestoreTeamActionPoint(TArray<ACustomThirdPerson*>& Characters);

	TArray<class APlayerDetector*> PlayerDetectors;

	UFUNCTION()
	void SetVisibleAllHealthBar(const bool bVisible);

	void SpawnFogOfWar();

	void StartBotActivity();

	void SetEnemysPatrolDirection();

	EDirection GetDirectionFromEnemyGroup(FVector GroupMiddlePoint, FVector DetectorLocation) const;

	int32 EnemyTurnOrder = 0;
};
