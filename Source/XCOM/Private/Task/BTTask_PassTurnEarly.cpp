// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_PassTurnEarly.h"
#include "EnemyUnit.h"
#include "EnemyController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"
#include "BehaviorTree/BehaviorTreeTypes.h"

UBTTask_PassTurnEarly::UBTTask_PassTurnEarly()
{
	NodeName = "PassTurnEarly";
}

EBTNodeResult::Type UBTTask_PassTurnEarly::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyController* BotController = Cast<AEnemyController>(OwnerComp.GetAIOwner());
	AEnemyUnit* ControlledUnit = BotController ? Cast<AEnemyUnit>(BotController->GetPawn()) : nullptr;
	if (!ControlledUnit)
	{
		return EBTNodeResult::Failed;
	}
	ControlledUnit->ForceOverTurn();
	return EBTNodeResult::Succeeded;
}
