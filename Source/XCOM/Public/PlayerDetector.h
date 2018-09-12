// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerDetector.generated.h"

UCLASS()
class XCOM_API APlayerDetector : public AActor
{
	GENERATED_BODY()
	
public:	
	APlayerDetector();

protected:
	virtual void BeginPlay() override;

	/**
	* Player Unit���� �Ÿ��� ����մϴ�.
	* @param PlayerUnitArray - �÷��̾� ������ �迭
	* @return Player Unit����� �Ÿ��� ��
	*/
	float SumOfDistancesToPlayerUnit(TArray<AActor*>& PlayerUnitArray) const;

public:	
	virtual void Tick(float DeltaTime) override;

	
	
};
