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
	* Player Unit과의 거리를 계산합니다.
	* @param PlayerUnitArray - 플레이어 유닛의 배열
	* @return Player Unit들과의 거리의 합
	*/
	float SumOfDistancesToPlayerUnit(TArray<AActor*>& PlayerUnitArray) const;

public:	
	virtual void Tick(float DeltaTime) override;

	
	
};
