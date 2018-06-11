// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_FollowThePath.h"
#include "CustomThirdPerson.h"
#include "EnemyController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"
#include "BehaviorTree/BehaviorTreeTypes.h"

EBTNodeResult::Type UBTTask_FollowThePath::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyController* BotController = Cast<AEnemyController>(OwnerComp.GetAIOwner());
	ACustomThirdPerson* ControlledUnit = BotController ? Cast<ACustomThirdPerson>(BotController->GetPawn()) : nullptr;
	if (!ControlledUnit) 
	{
		return EBTNodeResult::Failed;
	}

	if (ControlledUnit->GetSpeed() > 0) 
	{
		return EBTNodeResult::InProgress;
	}
	else 
	{
		BotController->FollowThePath();
		FBlackboard::FKey BlackboardKey = OwnerComp.GetBlackboardComponent()->GetKeyID("RemainingMovement");
		OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Bool>(BlackboardKey, false);
		return EBTNodeResult::Succeeded;
	}
	
}
