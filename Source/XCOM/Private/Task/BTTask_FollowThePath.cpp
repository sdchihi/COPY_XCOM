// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_FollowThePath.h"
#include "CustomThirdPerson.h"
#include "EnemyController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"
#include "BehaviorTree/BehaviorTreeTypes.h"

UBTTask_FollowThePath::UBTTask_FollowThePath()
{
	NodeName = "FollowThePath";
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_FollowThePath::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyController* BotController = Cast<AEnemyController>(OwnerComp.GetAIOwner());
	ACustomThirdPerson* ControlledUnit = BotController ? Cast<ACustomThirdPerson>(BotController->GetPawn()) : nullptr;
	if (!ControlledUnit) 
	{
		return EBTNodeResult::Failed;
	}
	BotController->FollowThePath();
	return EBTNodeResult::InProgress;
}



void UBTTask_FollowThePath::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AEnemyController* BotController = Cast<AEnemyController>(OwnerComp.GetAIOwner());
	ACustomThirdPerson* ControlledUnit = BotController ? Cast<ACustomThirdPerson>(BotController->GetPawn()) : nullptr;
	if (ControlledUnit->GetSpeed() == 0) 
	{
		FBlackboard::FKey BlackboardKey = OwnerComp.GetBlackboardComponent()->GetKeyID("RemainingMovement");
		OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Bool>(BlackboardKey, false);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}