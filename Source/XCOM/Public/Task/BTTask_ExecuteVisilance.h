// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_ExecuteVisilance.generated.h"

/**
 * 
 */
UCLASS()
class XCOM_API UBTTask_ExecuteVisilance : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;


public:
	UBTTask_ExecuteVisilance();

	
};
