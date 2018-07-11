// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_GetNextAction.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"
#include "CustomThirdPerson.h"


EBTNodeResult::Type UBTTask_GetNextAction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyController* BotController = Cast<AEnemyController>(OwnerComp.GetAIOwner());
	ACustomThirdPerson* MyBot = BotController ? Cast<ACustomThirdPerson>(BotController->GetPawn()) : NULL;
	if (MyBot == NULL)
	{
		UE_LOG(LogTemp, Warning, L"GetNextAction 실패")

		return EBTNodeResult::Failed;
	}
	BotController->SetNextAction();
	
	UE_LOG(LogTemp, Warning, L"GetNextAction 성공")

	return EBTNodeResult::Succeeded;
}
