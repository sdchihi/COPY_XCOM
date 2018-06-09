// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_SetNextLocation.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"
#include "CustomThirdPerson.h"



EBTNodeResult::Type UBTTask_SetNextLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyController* BotController = Cast<AEnemyController>(OwnerComp.GetAIOwner());
	ACustomThirdPerson* MyBot = BotController ? Cast<ACustomThirdPerson>(BotController->GetPawn()) : NULL;
	if (MyBot == NULL)
	{
		return EBTNodeResult::Failed;
	}


	return EBTNodeResult::Succeeded;
}
