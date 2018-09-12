// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoveringChecker.generated.h"

UCLASS()
class XCOM_API ACoveringChecker : public AActor
{
	GENERATED_BODY()
	
public:	
	ACoveringChecker();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, Category = "Instance Mesh")
	class UStaticMesh* CoverMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Instance Mesh")
	class UMaterial* FullCoverMaterial = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Instance Mesh")
	UMaterial*  FullCoverBorderMaterial = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Instance Mesh")
	UMaterial* HalfCoverMaterial = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Instance Mesh")
	UMaterial* HalfCoverBorderMaterial = nullptr;

	/**
	* ���󰡴��� ���� ���� Mesh�� �����մϴ�.
	* @param TileLocationArray - Cover Mesh������ �ĺ��� �� Ÿ�� ��ġ �迭
	* @param Spacing - �޽� ������ ���� ����
	*/
	void MakingCoverNotice(TArray<FVector>& TileLocationArray, float Spacing);

	/** ��� Covering Mesh�� �����մϴ�. */
	void ClearAllCoverNotice();

private:
	class UInstancedStaticMeshComponent* FullCoverInstancedMeshComp = nullptr;

	UInstancedStaticMeshComponent* HalfCoverInstancedMeshComp = nullptr;

	UMaterialInstanceDynamic* FullCoverMaterialDynamic;

	UMaterialInstanceDynamic* HalfCoverMaterialDynamic;

	/**
	* RayCast�� ����⿡ ������ Mesh���� ���� ��ġ�� �����ϴ�.
	* @param OriginLocation - Ÿ���� ��ġ
	* @param Spacing - �޽� ������ ���� ����
	* @param HalfCoverNoticeTF - �� ���� �޽��� ������ ��ġ �迭
	* @param FullCoverNoticeTF - ���� ���� �޽��� ������ ��ġ �迭
	*/
	void RayCastToCardinalDirection(FVector OriginLocation, float Spacing, OUT TArray<FTransform>& HalfCoverNoticeTF, OUT TArray<FTransform>& FullCoverNoticeTF);

	void AddFullCoverInstances(TArray<FTransform>& TransformArray) const;

	void AddHalfCoverInstances(TArray<FTransform>& TransformArray) const;


};
