// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_ExecuteVisilance.h"
#include "CustomThirdPerson.h"
#include "EnemyController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"
#include "BehaviorTree/BehaviorTreeTypes.h"


UBTTask_ExecuteVisilance::UBTTask_ExecuteVisilance()
{
	NodeName = "ExecuteVisilance";
}


EBTNodeResult::Type UBTTask_ExecuteVisilance::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyController* BotController = Cast<AEnemyController>(OwnerComp.GetAIOwner());
	ACustomThirdPerson* ControlledUnit = BotController ? Cast<ACustomThirdPerson>(BotController->GetPawn()) : nullptr;
	if (!ControlledUnit)
	{
		UE_LOG(LogTemp, Warning, L"임시 실패")

		return EBTNodeResult::Failed;
	}
	BotController->OrderStartVigilance();
	UE_LOG(LogTemp,Warning,L"임시 성공")

	return EBTNodeResult::Succeeded;
}

