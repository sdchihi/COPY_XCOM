// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_InfomalFinishTurn.h"
#include "CustomThirdPerson.h"
#include "EnemyController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"
#include "BehaviorTree/BehaviorTreeTypes.h"

UBTTask_InfomalFinishTurn::UBTTask_InfomalFinishTurn()
{
	NodeName = "InfomalFinishTurn";
}

EBTNodeResult::Type UBTTask_InfomalFinishTurn::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyController* BotController = Cast<AEnemyController>(OwnerComp.GetAIOwner());
	ACustomThirdPerson* ControlledUnit = BotController ? Cast<ACustomThirdPerson>(BotController->GetPawn()) : nullptr;
	if (!ControlledUnit)
	{
		return EBTNodeResult::Failed;
	}
	BotController->StopBehaviorTree();

	return EBTNodeResult::Succeeded;
}
