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

	void ChangeEnemyAggro(int8 EnemyGroupNumber);

	AFogOfWarManager* GetFowManager() { return FogOfWar; }

	bool GetEnemysBattleCognition() { return bEnemyNoticeBattle; };

	void SetEnemysBattleCognition() { bEnemyNoticeBattle = true; };

	FVector GetPlayerUnitMiddlePoint();

private:
	UPROPERTY()
	AFogOfWarManager* FogOfWar = nullptr;

	bool TurnBuffer = false;

	int32 EnemyTurnOrder = 0;

	bool bEnemyNoticeBattle = false;

	TArray<ACustomThirdPerson*> PlayerCharacters;

	TArray<ACustomThirdPerson*> EnemyCharacters;

	TArray<class AWaypoint*> WaypointArray;

	TMap<int8, TArray<class AEnemyUnit*>> EnemyTeamMap;
	
	TMap<int8, AWaypoint*> WaypointMap;

	void RestoreTeamActionPoint(TArray<ACustomThirdPerson*>& Characters);

	UFUNCTION()
	void SetVisibleAllHealthBar(const bool bVisible);

	void SpawnFogOfWar();

	void StartBotActivity();

	void SetEnemysPatrolLocation();

	void InitializeWaypointMap();

	void RenewWaypoint(int8 EnemyGroupNumber);

	AActor* EventUnit;

	bool HasEnemyUnitEvent();

	void ExecuteEnemyEvent();

	UFUNCTION()
	void RegisterEventActor(AActor* EventActor);

	UFUNCTION()
	void CheckTurnAfterEvent();
};
