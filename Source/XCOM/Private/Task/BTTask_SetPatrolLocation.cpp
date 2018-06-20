// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_SetPatrolLocation.h"
#include "EnemyController.h"
#include "EnemyUnit.h"



EBTNodeResult::Type UBTTask_SetPatrolLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyController* BotController = Cast<AEnemyController>(OwnerComp.GetAIOwner());
	AEnemyUnit* MyBot = BotController ? Cast<AEnemyUnit>(BotController->GetPawn()) : NULL;
	if (MyBot == NULL)
	{
		return EBTNodeResult::Failed;
	}
	BotController->SetNextPatrolLocation();

	return EBTNodeResult::Succeeded;
}
