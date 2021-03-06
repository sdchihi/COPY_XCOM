// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_FinishTurn.h"
#include "CustomThirdPerson.h"
#include "EnemyController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"
#include "BehaviorTree/BehaviorTreeTypes.h"


EBTNodeResult::Type UBTTask_FinishTurn::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyController* BotController = Cast<AEnemyController>(OwnerComp.GetAIOwner());
	ACustomThirdPerson* ControlledUnit = BotController ? Cast<ACustomThirdPerson>(BotController->GetPawn()) : nullptr;
	if (!ControlledUnit)
	{
		return EBTNodeResult::Failed;
	}
	UE_LOG(LogTemp, Warning, L"enemy unit �� ����")

	BotController->StopBehaviorTree();
	if (ControlledUnit->bCanAction) 
	{
		ControlledUnit->UseActionPoint(3);
	}

	return EBTNodeResult::Succeeded;
};


