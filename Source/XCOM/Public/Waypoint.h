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

	// ���� Waypoint
	UPROPERTY(EditInstanceOnly, Category = "Linked Waypoint")
	AWaypoint* PrevWaypoint = nullptr;

	// ���� Waypoint
	UPROPERTY(EditInstanceOnly, Category = "Linked Waypoint")
	AWaypoint* NextWaypoint = nullptr;

	bool IsForward() { return bForward; };

	// ��������� �����մϴ�.
	void TurnDirection() { bForward = !bForward; };
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
private: 
	// ���������� �����ϴ��� ����
	bool bForward = false;
	
};
