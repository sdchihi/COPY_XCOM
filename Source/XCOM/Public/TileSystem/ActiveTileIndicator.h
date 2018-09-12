// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ActiveTileIndicator.generated.h"

UCLASS()
class XCOM_API AActiveTileIndicator : public AActor
{
	GENERATED_BODY()
	
public:	
	AActiveTileIndicator();

	UPROPERTY(VisibleAnywhere)
	class UInstancedStaticMeshComponent* CloseTileMesh;

	UPROPERTY(VisibleAnywhere)
	class UInstancedStaticMeshComponent* DistantTileMsh;

	void ClearAllTile();

	/**
	* 활성화된 Tile들 위에 Mesh를 생성합니다.
	* @param CloseTileTransArray - 가까운 타일들의 Transform 목록
	* @param DistantTileTransArray - 먼 타일들의 Transform 목록
	*/
	void IndicateActiveTiles(TArray<FTransform> CloseTileTransArray, TArray<FTransform> DistantTileTransArray);

	void SetTileVisibility(bool Visible);
protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

private:

};
