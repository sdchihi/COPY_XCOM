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
	// Sets default values for this actor's properties
	ACoveringChecker();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, Category = "Instance Mesh")
	class UStaticMesh* CoverMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Instance Mesh")
	class UMaterial* FullCoverMaterial = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Instance Mesh")
	class UMaterial* HalfCoverMaterial = nullptr;


	void MakingCoverNotice(TArray<FVector>& TileLocationArray, float Spacing);

	void ClearAllCoverNotice();



private:
	class UInstancedStaticMeshComponent* FullCoverInstancedMeshComp = nullptr;

	UInstancedStaticMeshComponent* HalfCoverInstancedMeshComp = nullptr;

	UMaterialInstanceDynamic* FullCoverMaterialDynamic;

	UMaterialInstanceDynamic* HalfCoverMaterialDynamic;

	void RayCastToCardinalDirection(FVector OriginLocation, float Spacing, OUT TArray<FTransform>& HalfCoverNoticeTF, OUT TArray<FTransform>& FullCoverNoticeTF);

	void AddFullCoverInstances(TArray<FTransform>& TransformArray) const;

	void AddHalfCoverInstances(TArray<FTransform>& TransformArray) const;


};
