// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Unit/CustomThirdPerson.h"
#include "EnemyUnit.generated.h"

/**
 * 
 */
UCLASS()
class XCOM_API AEnemyUnit : public ACustomThirdPerson
{
	GENERATED_BODY()
public:
	int8 GetGroupNumber() { return Group; }

	bool IsAggro() { return bAggro; };

private:
	UPROPERTY(EditDefaultsOnly)
	int8 Group;
	

	bool bAggro = false;
};
