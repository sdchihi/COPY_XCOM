// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_AttackPlayerUnit.h"
#include "CustomThirdPerson.h"
#include "EnemyController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"
#include "BehaviorTree/BehaviorTreeTypes.h"

UBTTask_AttackPlayerUnit::UBTTask_AttackPlayerUnit()
{
	NodeName = "AttackPlayerUnit";
	bNotifyTick = true;
}


EBTNodeResult::Type UBTTask_AttackPlayerUnit::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyController* BotController = Cast<AEnemyController>(OwnerComp.GetAIOwner());
	ACustomThirdPerson* ControlledUnit = BotController ? Cast<ACustomThirdPerson>(BotController->GetPawn()) : nullptr;
	if (!ControlledUnit)
	{
		return EBTNodeResult::Failed;
	}

	BotController->ShootToPlayerUnit();

	return EBTNodeResult::InProgress;
}



void UBTTask_AttackPlayerUnit::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AEnemyController* BotController = Cast<AEnemyController>(OwnerComp.GetAIOwner());
	ACustomThirdPerson* ControlledUnit = BotController ? Cast<ACustomThirdPerson>(BotController->GetPawn()) : nullptr;
	if (!ControlledUnit->bIsAttack && !ControlledUnit->bIsReadyToAttack)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
