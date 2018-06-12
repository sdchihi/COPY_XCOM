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

	UPROPERTY(EditDefaultsOnly)
	class UStaticMesh* MeshToRegister = nullptr;

	void AddInstances(TArray<FTransform>& TransformArrray) const;


private:
	class UInstancedStaticMeshComponent* InstancedStaticMesh = nullptr;
	
};
