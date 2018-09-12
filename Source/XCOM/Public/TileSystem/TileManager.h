// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Path.h"
#include "TileManager.generated.h"

class ATile;
class UPrimitiveComponent;
class UInstancedStaticMeshComponent;

UCLASS()
class XCOM_API ATileManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ATileManager();

protected:
	virtual void BeginPlay() override;

public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class ACoveringChecker> CoveringCheckerBlueprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class APathIndicator> PathIndicatorBlueprint;

	/** ��θ� ���� �迭 ( length == Ÿ�� ���� )*/
	TArray<Path> PathArr;

	virtual void Tick(float DeltaTime) override;

	/**
	* �� Ÿ���� �������� �̵� ������ Ÿ�ϵ��� �����ϴ�.
	* @param StartingTile - �������̵� Pawn�� �ö��ִ� Ÿ��
	* @param MovingAbility - �ൿ�� ( �ִ� �̵� �Ÿ� )
	* @param AvailableTiles - �̵� ������ Ÿ�ϵ��� ��� Array
	*/
	void GetAvailableTiles(ATile* StartingTile, const int32 MovingAbility,int32 MovableStepsPerAct, OUT TArray<ATile*>& AvailableTiles);

	/**
	* Location�� �̿��� Tile�� Index�� �����ϴ�.
	* @param WorldLocation - Ÿ���� ���� ��ǥ
	* @return Ÿ���� Index�� ��ȯ�մϴ�.
	*/
	int32 ConvertVectorToIndex(const FVector WorldLocation);

	/**
	* Ÿ���� Index�� �̿��� ���� ��ǥ�� �����ϴ�.
	* @param Index - Ÿ���� �ε���
	* @return Ÿ���� ������ǥ�� ��ȯ�մϴ�.
	*/
	FVector ConvertIndexToVector(const int32 Index);

	void ConvertVectorToCoord(const FVector WorldLocation,OUT int& x,OUT int& y);

	bool IsSameLine(const int32 OverlappedTileIndex, const int RowNumber, const int32 TargetIndex);

	/**
	* Ÿ���� �ε����� �ִ� ������ �Ѿ���� Ȯ���մϴ�.
	* @param TileIndex - �˻��ϰ� �� Ÿ���� index
	* @return �ش� Ÿ���� ��ȿ ������ ������� ���θ� ��ȯ�մϴ�.
	*/
	bool CheckWithinBounds(const int32 TileIndex);

	/**
	* A* �˰��� �̿�Ǵ� �����͵��� �ʱ�ȭ�Ѵ�.
	* @param bClearAll - Path�������� ��� ������ �����ϴ� ����
	*/
	void ClearAllTiles(const bool bClearAll = false);


	int32 GetGridXLength() { return x; }

	/**
	* Ÿ������ ��Ÿ���� ȭ��ǥ Decal ������ �����ϰ�, ���ü��� �����մϴ�.
	* @param PathInfo - Key : Ÿ��(key)���� ���� Ÿ�Ϸ�  �̵�����(value)�� �����ִ� Map
	* @param bVisibility - Decal�� ������ ���� �����մϴ�.
	*/
	void SetDecalVisibilityOnTile(TMap<int32, float> PathInfo, const bool bVisibility);

	ATile* GetOverlappedTile(APawn* Pawn);

	/**
	* Ÿ�� �ֺ����� ����⿡ ���� �ִ��� ���θ� Ȯ���մϴ�.
	* @param TileLocation - Ÿ���� ��ġ
	* @param CoverDirectionArr - ���� ������ ����ִ� �迭�Դϴ�.
	* @return �ֺ��� ���� �ִ��� ����
	*/
	bool CheckWallAround(const FVector TileLocation, TArray<FVector>& CoverDirectionArr);

	int32 GetTileSize() { return TileSize; };

	Path& GetPathToTile(const int32 TileIndex) { return PathArr[TileIndex]; };

private:
	// ����
	ACoveringChecker* CoveringChecker = nullptr;

	APathIndicator* PathIndicator = nullptr;

	/** Ÿ�ϵ��� �迭 */
	TArray<ATile*> ChildTiles;

	/** Ÿ�� ������ */
	UPROPERTY(EditDefaultsOnly)
	int32 TileSize = 100;

	/** �� */
	UPROPERTY(EditDefaultsOnly)
	int32 x = 10;

	/** �� */
	UPROPERTY(EditDefaultsOnly)
	int32 y = 10;

	/** A* �˰��򿡼� Open list */
	TArray<int32> OpenList;

	/** A* �˰��򿡼� Closed list */
	TArray<int32> ClosedList;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root = nullptr;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ATile> TileBlueprint;

	/** ���� ���� ��ġ�� Ÿ�� �迭 */
	TArray<int32> TileIndexInRange;

	UFUNCTION()
	void MouseOnTile(UPrimitiveComponent* OverlappedComponent);

	UFUNCTION()
	void EndMouseOnTile(UPrimitiveComponent* OverlappedComponent);

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void EndTileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/**
	* Ÿ�ϰ� ����ź �Ÿ��� �����ϴ�.
	* @param StartIndex - ���� Ÿ�� �ε���
	* @param TargetIndex - ��ǥ Ÿ�� �ε���
	* @return ����ź �Ÿ��� ��ȯ�մϴ�.
	*/
	int32 ComputeManhattanDistance(const int32 StartIndex, const int32 TargetIndex);

	/**
	* Ÿ������ Wall Actor�� Ȯ���մϴ�.
	* @parma TileActor - Tile Actor Pointer
	*/
	void FindingWallOnTile(ATile* TileActor);

	/**
	* �̵� �������� �ִ� �� Ÿ�ϵ鿡 ���ؼ� A* �˰����� ������ ���� �̵������� Ÿ�ϵ��� ����� �����ϴ�.
	* @param StartingIndex - �������̵� Pawn�� �ö��ִ� Ÿ��
	* @param MovingAbility - �ൿ�� ( �ִ� �̵� �Ÿ� )
	* @param TileIndexInRange - �̵� �������� �����ϴ� Ÿ�ϵ�
	* @return ���� �̵������� Ÿ�ϵ��� Array
	*/
	TArray<ATile*> FindPath(const int32 StartingIndex, const int32 MovingAbility, int32 MovableStepsPerAct, TArray<int32> TileIndexInRange);

	/**
	* A* �˰��� ���� ���� ã���ϴ�.
	* @param CurrentIndex - ��ã�� �������� Ÿ�� �ε���
	* @param StartIndex - ���� ������ Ÿ�� �ε���
	* @param TargetIndex - ��ǥ ������ Ÿ�� �ε���
	* @return ��ǥ ������ Ÿ�Ϸ� �̵����� ���θ� ��ȯ�մϴ�.
	*/
	bool UpdatePathInfo(const int32 CurrentIndex, const int32 StartIndex, const int32 TargetIndex);

	/**
	* ��� Cardinal ���⿡ ���ؼ� A*�˰����� �����մϴ�.
	* @param CurrentIndex - ��ã�� �������� Ÿ�� �ε���
	* @param TargetIndex - ��ǥ ������ Ÿ�� �ε���
	*/
	void UpdateCardinalPath(const int32 CurrentIndex, const int32 TargetIndex);

	/**
	* �ϳ��� Cadinal ������ Tile�� ���ؼ� ��� ����� �մϴ�.
	* @param CurrentIndex - ��ã�� �������� Ÿ�� �ε���
	* @param CardinalPathIndex - ����꿡 �̿�� Cardinal ������ Ÿ�� �ε���
	* @param TargetIndex - ��ǥ ������ Ÿ�� �ε���
	*/
	void UpdateOneCardinalPath(const int32 CurrentIndex, const int32 CardinalPathIndex, const int32 TargetIndex);

	/**
	* ��� Diagonal ���⿡ ���ؼ� A*�˰����� �����մϴ�.
	* @param CurrentIndex - ��ã�� �������� Ÿ�� �ε���
	* @param TargetIndex - ��ǥ ������ Ÿ�� �ε���
	*/
	void UpdateDiagonalPath(const int32 CurrentIndex, const int32 TargetIndex);

	/**
	* �ϳ��� Diagonal ������ Tile�� ���ؼ� ��� ����� �մϴ�.
	* @param CurrentIndex - ��ã�� �������� Ÿ�� �ε���
	* @param DiagonalPathIndex - ����꿡 �̿�� Diagonal ������ Ÿ�� �ε���
	* @param TargetIndex - ��ǥ ������ Ÿ�� �ε���
	*/
	void UpdateOneDiagonalPath(const int32 CurrentIndex, const int32 DiagonalPathIndex, const int32 TargetIndex);

	/**
	* F ����� ���� ���� ���� Index�� �����ϴ�
	* @return F ����� ���� ���� ���� Index
	*/
	int32 FindMinCostFIndex();

	/**
	* Ÿ�� �ֺ��� ���� �ִ��� ���θ� Ȯ���մϴ�.
	* @param TileIndex - Ÿ���� Index
	* @param CardinalIndex - Ȯ���� Cardinal ���⿡ �ִ� Ÿ���� Index
	* @param CoverDirectionArr - ���� ������ ����ִ� �迭�Դϴ�.
	*/
	void CheckWallAroundOneDirection(const int32 TileIndex, const int CardinalIndex, TArray<FVector>& CoverDirectionArr);

	/**
	* Ÿ���� �̵��������� ���θ� Ȯ���մϴ�.
	* @param TileIndex - Ȯ���� Ÿ���� �ε���
	* @return �̵� ���� ����
	*/
	bool CheckAvailability(int32 TileIndex);

	/**
	* �����Ҽ� �ִ� ���� ���� Mesh�� �����մϴ�.
	* @param OriginTileIndex - ���� �ִ� Ÿ���� �ε���
	*/
	void MakingCoverNotice(int32 OriginTileIndex);
};
