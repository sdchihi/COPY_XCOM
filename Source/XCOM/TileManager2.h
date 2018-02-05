// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TileManager2.generated.h"

class UTile;
class UInstancedStaticMeshComponent;

struct Path {
	bool bWall;
	int32 Cost;
};

UCLASS()
class XCOM_API ATileManager2 : public AActor
{
	GENERATED_BODY()
	
public:	
	ATileManager2();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditDefaultsOnly)
	int32 TileSize = 100;

	UPROPERTY(EditDefaultsOnly)
	int32 x = 10;

	UPROPERTY(EditDefaultsOnly)
	int32 y = 10;
	
	TArray<Path> PathArr;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root = nullptr;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> WallBlueprint;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	int32 ConvertVectorToIndex(FVector WorldLocation);

	FVector ConvertIndexToVector(int32 Index);

	void FindingWallOnTile(AActor* TileActor);

};
