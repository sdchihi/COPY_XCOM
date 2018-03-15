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

	void GetAvailableTiles(ATile* StartingTile, const int32 MovingAbility, TArray<ATile*>& AvailableTiles);

	int32 ConvertVectorToIndex(const FVector WorldLocation);

	FVector ConvertIndexToVector(const int32 Index);

	void ConvertVectorToCoord(const FVector WorldLocation,OUT int& x,OUT int& y);

	bool IsSameLine(const int32 OverlappedTileIndex, const int RowNumber, const int32 TargetIndex);
	
	bool CheckWithinBounds(const int32 TileIndex);

	void ClearAllTiles(const bool bClearAll = false);

	TArray<Path> PathArr;

	int32 GetGridXLength() { return x; }

	void SetDecalVisibilityOnTile(TMap<int32, float> PathInfo, const int32 NumberOfTimes, const bool bVisibility);

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


	int32 ComputeManhattanDistance(const int32 StartIndex, const int32 TargetIndex);

	void FindingWallOnTile(ATile* TileActor);


	TArray<ATile*> FindPath(const int32 StartingIndex, const int32 MovingAbility, TArray<int32> TileIndexInRange);

	bool UpdatePathInfo(const int32 CurrentIndex, const int32 StartIndex, const int32 TargetIndex);

	void UpdateCardinalPath(const int32 CurrentIndex, const int32 TargetIndex);

	void UpdateOneCardinalPath(const int32 CurrentIndex, const int32 CardinalPathIndex, const int32 TargetIndex);

	void UpdateDiagonalPath(const int32 CurrentIndex, const int32 TargetIndex);

	void UpdateOneDiagonalPath(const int32 CurrentIndex, const int32 DiagonalPathIndex, const int32 TargetIndex);

	int32 FindMinCostFIndex();

	
};
