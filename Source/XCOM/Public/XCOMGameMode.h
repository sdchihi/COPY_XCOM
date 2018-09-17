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

	/**
	* 팀의 턴의 끝났는지 확인합니다.
	* @param bIsPlayerTeam - 확인할 팀의 플래그
	*/
	UFUNCTION()
	void CheckTurnOver(const bool bIsPlayerTeam);

	UPROPERTY(EditInstanceOnly)
	TArray<AActor*> PlayerUnitDetectorArray;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UFloatingWidget> PopUpBlueprint;

	UPROPERTY(EditDefaultsOnly, meta = (ToolTip = "Fog of war"))
	TSubclassOf<AFogOfWarManager> FogOfWarBP;

	/** Fog of war 생성 여부 */
	UPROPERTY(EditDefaultsOnly, meta = (ToolTip = "Fog of war"))
	bool bGenerateFogOfWar = true;

	/**
	* 한쪽 팀의 모든 유닛을 얻어옵니다.
	* @param bTeam - 얻어올 팀의 플래그 ( 플레이어측 / AI 측 ) 
	* @return 한 팀의 모든 유닛을 담은 배열
	*/
	TArray<ACustomThirdPerson*> GetTeamMemeber(const bool bTeam);

	/**
	* 한 그룹의 AI Unit의 Aggro를 변경합니다.
	* @param EnemyGroupNumber - Aggro를 변경할 적 Unit들의 그룹 번호
	*/
	void ChangeEnemyAggro(int8 EnemyGroupNumber);

	AFogOfWarManager* GetFowManager() { return FogOfWar; }

	bool GetEnemysBattleCognition() { return bEnemyNoticeBattle; };

	void SetEnemysBattleCognition() { bEnemyNoticeBattle = true; };

	/**
	* 플레이어 유닛들의 중간 지점을 얻어옵니다.
	* @return 플레이어 유닛들의 중간 지점
	*/
	FVector GetPlayerUnitMiddlePoint();

private:
	UPROPERTY()
	UFloatingWidget* PopUp = nullptr;

	UPROPERTY()
	AFogOfWarManager* FogOfWar = nullptr;

	/**
	* 플레이어 유닛들의 중간 지점을 얻어옵니다.
	* @return 플레이어 유닛들의 중간 지점
	*/
	bool TurnBuffer = false;

	/** Enemy Unit의 차례를 제어합니다.*/
	int32 EnemyTurnOrder = 0;

	/** 전투가 벌어졌는지 여부*/
	bool bEnemyNoticeBattle = false;

	/** 플레이어 측 유닛 배열*/
	TArray<ACustomThirdPerson*> PlayerCharacters;

	/** AI 측 유닛 배열 */
	TArray<ACustomThirdPerson*> EnemyCharacters;

	/** 레벨에 배치된 정찰을 위한 Waypoint 배열 */
	TArray<class AWaypoint*> WaypointArray;

	/** 팀 별로 Unit을 담은 맵*/
	TMap<int8, TArray<class AEnemyUnit*>> EnemyTeamMap;
	
	TMap<int8, AWaypoint*> WaypointMap;

	/* 이벤트(주로 감정 표현)을 실행할 유닛 */
	AActor* EventUnit;
	
	/**
	* 캐릭터들의 Action point를 회복시킵니다.
	* @param Characters - 회복시킬 캐릭터들 배열
	*/
	UFUNCTION()
	void RestoreTeamActionPoint(TArray<ACustomThirdPerson*>& Characters);

	/**
	* 캐릭터들의 Health bar의 가시성을 변경합니다.
	* @param bVisible
	*/
	UFUNCTION()
	void SetVisibleAllHealthBar(const bool bVisible);
	
	/** FOW(전장의 안개)를 생성합니다.*/
	void SpawnFogOfWar();

	/** 한 AI Unit의 활동을 시작합니다.*/
	void StartBotActivity();

	/** AI Unit의 정찰 위치를 결정합니다.*/
	void SetEnemysPatrolLocation();

	/* Enemy Group 별로 정찰을 하게될 Waypoint를 Map으로 구성합니다 */
	void InitializeWaypointMap();

	/**
	* Waypoint의 연결 상태에 따라 다음 진행 Waypoint로 맵을 갱신시킵니다.
	* @param EnemyGroupNumber
	*/
	void RenewWaypoint(int8 EnemyGroupNumber);


	/**
	* 특정 이벤트(감정 표현)의 실행을 기다리는 유닛이 있는지 확인합니다.
	* @return 기다리는 유닛이 있는지 여부
	*/
	bool HasEnemyUnitEvent();

	/** 특정 이벤트(감정 표현) 실행 명령을 내립니다. */
	void ExecuteEnemyEvent();

	/** 특정 이벤트(감정 표현)를 실행할 유닛을 등록합니다. */
	UFUNCTION()
	void RegisterEventActor(AActor* EventActor);

	UFUNCTION()
	void CheckTurnAfterEvent();

	/**
	* 전투 결과를 알리는 팝업을 띄웁니다.
	* @param DamagedActor - 데미지를 입은 Actor
	* @param Damage - 적용되는 데미지
	* @param State - 전투 결과를 알리는 State
	*/
	UFUNCTION()
	void ShowCombatPopUp(AActor* DamagedActor, float Damage, FloatingWidgetState State);

	/**
	* GameMode에서 해당 유닛을 해지합니다.
	* @param Unit - 해지할 Actor
	*/
	UFUNCTION()
	void UnRegisterUnit(ACustomThirdPerson* Unit);

	/** Player측의 턴을 종료합니다. */
	UFUNCTION()
	void PlayerTurnOver();

	/** AI측의 턴을 종료합니다.*/
	void EnemyTurnOver();

};
