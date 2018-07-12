// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_InfomalFinishTurn.generated.h"

/**
 * 
 */
UCLASS()
class XCOM_API UBTTask_InfomalFinishTurn : public UBTTaskNode
{
	GENERATED_BODY()
	
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;


public:
	UBTTask_InfomalFinishTurn();

	
};
