// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PathIndicator.generated.h"

UCLASS()
class XCOM_API APathIndicator : public AActor
{
	GENERATED_BODY()
	
public:	
	APathIndicator();

	UPROPERTY(VisibleAnywhere)
	class UInstancedStaticMeshComponent* DirectionMesh;

	void ClearAllTile();

	/**
	* �̵��� ������� Ÿ������ ǥ���մϴ�.
	* @param PathTransform - �̵� ������ ǥ���� Transform �迭
	*/
	void IndicateDirection(TArray<FTransform> PathTransform);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	
};
