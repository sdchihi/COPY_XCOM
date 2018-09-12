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
	* 엄폐가능한 벽에 엄폐 Mesh를 생성합니다.
	* @param TileLocationArray - Cover Mesh생성의 후보가 될 타일 위치 배열
	* @param Spacing - 메쉬 생성을 위한 간격
	*/
	void MakingCoverNotice(TArray<FVector>& TileLocationArray, float Spacing);

	/** 모든 Covering Mesh를 제거합니다. */
	void ClearAllCoverNotice();

private:
	class UInstancedStaticMeshComponent* FullCoverInstancedMeshComp = nullptr;

	UInstancedStaticMeshComponent* HalfCoverInstancedMeshComp = nullptr;

	UMaterialInstanceDynamic* FullCoverMaterialDynamic;

	UMaterialInstanceDynamic* HalfCoverMaterialDynamic;

	/**
	* RayCast를 사방향에 보내서 Mesh생성 가능 위치를 얻어냅니다.
	* @param OriginLocation - 타일의 위치
	* @param Spacing - 메쉬 생성을 위한 간격
	* @param HalfCoverNoticeTF - 반 엄폐 메쉬를 생성할 위치 배열
	* @param FullCoverNoticeTF - 완전 엄폐 메쉬를 생성할 위치 배열
	*/
	void RayCastToCardinalDirection(FVector OriginLocation, float Spacing, OUT TArray<FTransform>& HalfCoverNoticeTF, OUT TArray<FTransform>& FullCoverNoticeTF);

	void AddFullCoverInstances(TArray<FTransform>& TransformArray) const;

	void AddHalfCoverInstances(TArray<FTransform>& TransformArray) const;


};
