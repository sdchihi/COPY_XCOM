// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "XCOMGameMode.generated.h"

enum class FloatingWidgetState : uint8;

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
	~AXCOMGameMode();


	UFUNCTION()
	void CheckTurnOver(const bool bIsPlayerTeam);

	UPROPERTY(EditInstanceOnly)
	TArray<AActor*> PlayerUnitDetectorArray;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UFloatingWidget> PopUpBlueprint;

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
	UFloatingWidget* PopUp = nullptr;

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

	UFUNCTION()
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

	UFUNCTION()
	void ShowCombatPopUp(AActor* DamagedActor, float Damage, FloatingWidgetState State);

	UFUNCTION()
	void UnRegisterUnit(ACustomThirdPerson* Unit);

	UFUNCTION()
	void PlayerTurnOver();


	void EnemyTurnOver();

};
