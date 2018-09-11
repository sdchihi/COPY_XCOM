// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "PawnController.generated.h"

class ACustomThirdPerson;
/**
 * 
 */
UCLASS()
class XCOM_API APawnController : public AAIController
{
	GENERATED_BODY()
	
	
public:

	virtual void BeginPlay() override;


};
