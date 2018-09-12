// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Waypoint.generated.h"

UCLASS()
class XCOM_API AWaypoint : public AActor
{
	GENERATED_BODY()
	
public:	
	AWaypoint();

	// 이전 Waypoint
	UPROPERTY(EditInstanceOnly, Category = "Linked Waypoint")
	AWaypoint* PrevWaypoint = nullptr;

	// 다음 Waypoint
	UPROPERTY(EditInstanceOnly, Category = "Linked Waypoint")
	AWaypoint* NextWaypoint = nullptr;

	bool IsForward() { return bForward; };

	// 진행방향을 변경합니다.
	void TurnDirection() { bForward = !bForward; };
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
private: 
	// 정방향으로 진행하는지 여부
	bool bForward = false;
	
};
