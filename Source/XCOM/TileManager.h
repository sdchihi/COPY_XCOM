// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TileManager.generated.h"


//struct Path {
//	bool bWall;
//	int32 Cost;
//};

UCLASS()
class XCOM_API ATileManager : public AActor
{
	GENERATED_BODY()
		
public:	
	// Sets default values for this actor's properties
	ATileManager();




protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RootMesh;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


private:

	UPROPERTY(EditDefaultsOnly)
	int32 TileSize = 1;
	
	UPROPERTY(EditDefaultsOnly)
	int32 x = 10;

	UPROPERTY(EditDefaultsOnly)
	int32 y = 10;
	
	//TArray<Path> PathArr;

	TArray<AActor*> OverlappedActors;
};
