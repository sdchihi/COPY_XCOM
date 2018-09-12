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
	* 이동할 방향들을 타일위에 표시합니다.
	* @param PathTransform - 이동 방향을 표시할 Transform 배열
	*/
	void IndicateDirection(TArray<FTransform> PathTransform);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	
};
