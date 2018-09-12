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
	* Ȱ��ȭ�� Tile�� ���� Mesh�� �����մϴ�.
	* @param CloseTileTransArray - ����� Ÿ�ϵ��� Transform ���
	* @param DistantTileTransArray - �� Ÿ�ϵ��� Transform ���
	*/
	void IndicateActiveTiles(TArray<FTransform> CloseTileTransArray, TArray<FTransform> DistantTileTransArray);

	void SetTileVisibility(bool Visible);
protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

private:

};
