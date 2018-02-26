// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Path.h"
#include "TileManager2.generated.h"

class ATile;
class UPrimitiveComponent;
class UInstancedStaticMeshComponent;



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

	void GetAvailableTiles(ATile* StartingTile, int32 MovingAbility, TArray<ATile*>& AvailableTiles);

	int32 ConvertVectorToIndex(FVector WorldLocation);

	FVector ConvertIndexToVector(int32 Index);

	void ConvertVectorToCoord(FVector WorldLocation,OUT int& x,OUT int& y);

	bool IsSameLine(int32 OverlappedTileIndex, int RowNumber, int32 TargetIndex);
	
	bool CheckWithinBounds(int32 TileIndex);

	void ClearAllTiles(bool bClearAll = false);

	TArray<Path> PathArr;

	int32 GetGridXLength() { return x; }

	void SetDecalVisibilityOnTile(TArray<int32> PathIndices, int32 NumberOfTimes, bool bVisibility);


	UFUNCTION()
	void MouseOnTile(UPrimitiveComponent* OverlappedComponent);

	UFUNCTION()
	void EndMouseOnTile(UPrimitiveComponent* OverlappedComponent);

private:
	// 변수

	UPROPERTY(EditDefaultsOnly)
	int32 TileSize = 100;

	UPROPERTY(EditDefaultsOnly)
	int32 x = 10;

	UPROPERTY(EditDefaultsOnly)
	int32 y = 10;
	

	TArray<int32> OpenList;

	TArray<int32> ClosedList;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root = nullptr;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ATile> TileBlueprint;

	TArray<int32> TileIndexInRange;
	//메소드


	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


	int32 ComputeManhattanDistance(int32 StartIndex, int32 TargetIndex);

	void FindingWallOnTile(ATile* TileActor);


	TArray<ATile*> FindPath(int32 StartingIndex,int32 MovingAbility, TArray<int32> TileIndexInRange);

	bool UpdatePathInfo(int32 CurrentIndex, int32 StartIndex, int32 TargetIndex);

	void UpdateCardinalPath(int32 CurrentIndex, int32 TargetIndex);

	void UpdateOneCardinalPath(int32 CurrentIndex, int32 CardinalPathIndex, int32 TargetIndex);

	void UpdateDiagonalPath(int32 CurrentIndex, int32 TargetIndex);

	void UpdateOneDiagonalPath(int32 CurrentIndex, int32 DiagonalPathIndex, int32 TargetIndex);

	int32 FindMinCostFIndex();

	
};
