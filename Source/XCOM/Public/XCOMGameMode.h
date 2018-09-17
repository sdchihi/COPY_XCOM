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
	* ���� ���� �������� Ȯ���մϴ�.
	* @param bIsPlayerTeam - Ȯ���� ���� �÷���
	*/
	UFUNCTION()
	void CheckTurnOver(const bool bIsPlayerTeam);

	UPROPERTY(EditInstanceOnly)
	TArray<AActor*> PlayerUnitDetectorArray;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UFloatingWidget> PopUpBlueprint;

	UPROPERTY(EditDefaultsOnly, meta = (ToolTip = "Fog of war"))
	TSubclassOf<AFogOfWarManager> FogOfWarBP;

	/** Fog of war ���� ���� */
	UPROPERTY(EditDefaultsOnly, meta = (ToolTip = "Fog of war"))
	bool bGenerateFogOfWar = true;

	/**
	* ���� ���� ��� ������ ���ɴϴ�.
	* @param bTeam - ���� ���� �÷��� ( �÷��̾��� / AI �� ) 
	* @return �� ���� ��� ������ ���� �迭
	*/
	TArray<ACustomThirdPerson*> GetTeamMemeber(const bool bTeam);

	/**
	* �� �׷��� AI Unit�� Aggro�� �����մϴ�.
	* @param EnemyGroupNumber - Aggro�� ������ �� Unit���� �׷� ��ȣ
	*/
	void ChangeEnemyAggro(int8 EnemyGroupNumber);

	AFogOfWarManager* GetFowManager() { return FogOfWar; }

	bool GetEnemysBattleCognition() { return bEnemyNoticeBattle; };

	void SetEnemysBattleCognition() { bEnemyNoticeBattle = true; };

	/**
	* �÷��̾� ���ֵ��� �߰� ������ ���ɴϴ�.
	* @return �÷��̾� ���ֵ��� �߰� ����
	*/
	FVector GetPlayerUnitMiddlePoint();

private:
	UPROPERTY()
	UFloatingWidget* PopUp = nullptr;

	UPROPERTY()
	AFogOfWarManager* FogOfWar = nullptr;

	/**
	* �÷��̾� ���ֵ��� �߰� ������ ���ɴϴ�.
	* @return �÷��̾� ���ֵ��� �߰� ����
	*/
	bool TurnBuffer = false;

	/** Enemy Unit�� ���ʸ� �����մϴ�.*/
	int32 EnemyTurnOrder = 0;

	/** ������ ���������� ����*/
	bool bEnemyNoticeBattle = false;

	/** �÷��̾� �� ���� �迭*/
	TArray<ACustomThirdPerson*> PlayerCharacters;

	/** AI �� ���� �迭 */
	TArray<ACustomThirdPerson*> EnemyCharacters;

	/** ������ ��ġ�� ������ ���� Waypoint �迭 */
	TArray<class AWaypoint*> WaypointArray;

	/** �� ���� Unit�� ���� ��*/
	TMap<int8, TArray<class AEnemyUnit*>> EnemyTeamMap;
	
	TMap<int8, AWaypoint*> WaypointMap;

	/* �̺�Ʈ(�ַ� ���� ǥ��)�� ������ ���� */
	AActor* EventUnit;
	
	/**
	* ĳ���͵��� Action point�� ȸ����ŵ�ϴ�.
	* @param Characters - ȸ����ų ĳ���͵� �迭
	*/
	UFUNCTION()
	void RestoreTeamActionPoint(TArray<ACustomThirdPerson*>& Characters);

	/**
	* ĳ���͵��� Health bar�� ���ü��� �����մϴ�.
	* @param bVisible
	*/
	UFUNCTION()
	void SetVisibleAllHealthBar(const bool bVisible);
	
	/** FOW(������ �Ȱ�)�� �����մϴ�.*/
	void SpawnFogOfWar();

	/** �� AI Unit�� Ȱ���� �����մϴ�.*/
	void StartBotActivity();

	/** AI Unit�� ���� ��ġ�� �����մϴ�.*/
	void SetEnemysPatrolLocation();

	/* Enemy Group ���� ������ �ϰԵ� Waypoint�� Map���� �����մϴ� */
	void InitializeWaypointMap();

	/**
	* Waypoint�� ���� ���¿� ���� ���� ���� Waypoint�� ���� ���Ž�ŵ�ϴ�.
	* @param EnemyGroupNumber
	*/
	void RenewWaypoint(int8 EnemyGroupNumber);


	/**
	* Ư�� �̺�Ʈ(���� ǥ��)�� ������ ��ٸ��� ������ �ִ��� Ȯ���մϴ�.
	* @return ��ٸ��� ������ �ִ��� ����
	*/
	bool HasEnemyUnitEvent();

	/** Ư�� �̺�Ʈ(���� ǥ��) ���� ����� �����ϴ�. */
	void ExecuteEnemyEvent();

	/** Ư�� �̺�Ʈ(���� ǥ��)�� ������ ������ ����մϴ�. */
	UFUNCTION()
	void RegisterEventActor(AActor* EventActor);

	UFUNCTION()
	void CheckTurnAfterEvent();

	/**
	* ���� ����� �˸��� �˾��� ���ϴ�.
	* @param DamagedActor - �������� ���� Actor
	* @param Damage - ����Ǵ� ������
	* @param State - ���� ����� �˸��� State
	*/
	UFUNCTION()
	void ShowCombatPopUp(AActor* DamagedActor, float Damage, FloatingWidgetState State);

	/**
	* GameMode���� �ش� ������ �����մϴ�.
	* @param Unit - ������ Actor
	*/
	UFUNCTION()
	void UnRegisterUnit(ACustomThirdPerson* Unit);

	/** Player���� ���� �����մϴ�. */
	UFUNCTION()
	void PlayerTurnOver();

	/** AI���� ���� �����մϴ�.*/
	void EnemyTurnOver();

};
