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

	/** 경로를 담을 배열 ( length == 타일 갯수 )*/
	TArray<Path> PathArr;

	virtual void Tick(float DeltaTime) override;

	/**
	* 한 타일을 기준으로 이동 가능한 타일들을 얻어냅니다.
	* @param StartingTile - 시작점이될 Pawn이 올라가있는 타일
	* @param MovingAbility - 행동력 ( 최대 이동 거리 )
	* @param AvailableTiles - 이동 가능한 타일들이 담길 Array
	*/
	void GetAvailableTiles(ATile* StartingTile, const int32 MovingAbility,int32 MovableStepsPerAct, OUT TArray<ATile*>& AvailableTiles);

	/**
	* Location을 이용해 Tile의 Index를 얻어냅니다.
	* @param WorldLocation - 타일의 월드 좌표
	* @return 타일의 Index를 반환합니다.
	*/
	int32 ConvertVectorToIndex(const FVector WorldLocation);

	/**
	* 타일의 Index를 이용해 월드 좌표를 얻어냅니다.
	* @param Index - 타일의 인덱스
	* @return 타일의 월드좌표를 반환합니다.
	*/
	FVector ConvertIndexToVector(const int32 Index);

	void ConvertVectorToCoord(const FVector WorldLocation,OUT int& x,OUT int& y);

	bool IsSameLine(const int32 OverlappedTileIndex, const int RowNumber, const int32 TargetIndex);

	/**
	* 타일의 인덱스가 최대 범위를 넘어가는지 확인합니다.
	* @param TileIndex - 검사하게 될 타일의 index
	* @return 해당 타일이 유효 범위를 벗어나는지 여부를 반환합니다.
	*/
	bool CheckWithinBounds(const int32 TileIndex);

	/**
	* A* 알고리즘에 이용되는 데이터들을 초기화한다.
	* @param bClearAll - Path정보까지 모두 지울지 결정하는 변수
	*/
	void ClearAllTiles(const bool bClearAll = false);


	int32 GetGridXLength() { return x; }

	/**
	* 타일위에 나타나는 화살표 Decal 방향을 변경하고, 가시성을 제어합니다.
	* @param PathInfo - Key : 타일(key)에서 다음 타일로  이동방향(value)을 갖고있는 Map
	* @param bVisibility - Decal을 보일지 말지 결정합니다.
	*/
	void SetDecalVisibilityOnTile(TMap<int32, float> PathInfo, const bool bVisibility);

	ATile* GetOverlappedTile(APawn* Pawn);

	/**
	* 타일 주변으로 사방향에 벽이 있는지 여부를 확인합니다.
	* @param TileLocation - 타일의 위치
	* @param CoverDirectionArr - 엄폐 방향을 담고있는 배열입니다.
	* @return 주변에 벽이 있는지 여부
	*/
	bool CheckWallAround(const FVector TileLocation, TArray<FVector>& CoverDirectionArr);

	int32 GetTileSize() { return TileSize; };

	Path& GetPathToTile(const int32 TileIndex) { return PathArr[TileIndex]; };

private:
	// 변수
	ACoveringChecker* CoveringChecker = nullptr;

	APathIndicator* PathIndicator = nullptr;

	/** 타일들의 배열 */
	TArray<ATile*> ChildTiles;

	/** 타일 사이즈 */
	UPROPERTY(EditDefaultsOnly)
	int32 TileSize = 100;

	/** 열 */
	UPROPERTY(EditDefaultsOnly)
	int32 x = 10;

	/** 행 */
	UPROPERTY(EditDefaultsOnly)
	int32 y = 10;

	/** A* 알고리즘에서 Open list */
	TArray<int32> OpenList;

	/** A* 알고리즘에서 Closed list */
	TArray<int32> ClosedList;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root = nullptr;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ATile> TileBlueprint;

	/** 범위 내에 위치한 타일 배열 */
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
	* 타일간 맨하탄 거리를 얻어냅니다.
	* @param StartIndex - 시작 타일 인덱스
	* @param TargetIndex - 목표 타일 인덱스
	* @return 맨하탄 거리를 반환합니다.
	*/
	int32 ComputeManhattanDistance(const int32 StartIndex, const int32 TargetIndex);

	/**
	* 타일위에 Wall Actor를 확인합니다.
	* @parma TileActor - Tile Actor Pointer
	*/
	void FindingWallOnTile(ATile* TileActor);

	/**
	* 이동 범위내에 있는 각 타일들에 대해서 A* 알고리즘을 적용해 실제 이동가능한 타일들의 목록을 얻어냅니다.
	* @param StartingIndex - 시작점이될 Pawn이 올라가있는 타일
	* @param MovingAbility - 행동력 ( 최대 이동 거리 )
	* @param TileIndexInRange - 이동 범위내에 존재하는 타일들
	* @return 실제 이동가능한 타일들의 Array
	*/
	TArray<ATile*> FindPath(const int32 StartingIndex, const int32 MovingAbility, int32 MovableStepsPerAct, TArray<int32> TileIndexInRange);

	/**
	* A* 알고리즘에 따라 길을 찾습니다.
	* @param CurrentIndex - 길찾기 진행중인 타일 인덱스
	* @param StartIndex - 시작 지점의 타일 인덱스
	* @param TargetIndex - 목표 지점의 타일 인덱스
	* @return 목표 지점의 타일로 이동가능 여부를 반환합니다.
	*/
	bool UpdatePathInfo(const int32 CurrentIndex, const int32 StartIndex, const int32 TargetIndex);

	/**
	* 모든 Cardinal 방향에 대해서 A*알고리즘을 적용합니다.
	* @param CurrentIndex - 길찾기 진행중인 타일 인덱스
	* @param TargetIndex - 목표 지점의 타일 인덱스
	*/
	void UpdateCardinalPath(const int32 CurrentIndex, const int32 TargetIndex);

	/**
	* 하나의 Cadinal 방향의 Tile에 대해서 비용 계산을 합니다.
	* @param CurrentIndex - 길찾기 진행중인 타일 인덱스
	* @param CardinalPathIndex - 비용계산에 이용될 Cardinal 방향의 타일 인덱스
	* @param TargetIndex - 목표 지점의 타일 인덱스
	*/
	void UpdateOneCardinalPath(const int32 CurrentIndex, const int32 CardinalPathIndex, const int32 TargetIndex);

	/**
	* 모든 Diagonal 방향에 대해서 A*알고리즘을 적용합니다.
	* @param CurrentIndex - 길찾기 진행중인 타일 인덱스
	* @param TargetIndex - 목표 지점의 타일 인덱스
	*/
	void UpdateDiagonalPath(const int32 CurrentIndex, const int32 TargetIndex);

	/**
	* 하나의 Diagonal 방향의 Tile에 대해서 비용 계산을 합니다.
	* @param CurrentIndex - 길찾기 진행중인 타일 인덱스
	* @param DiagonalPathIndex - 비용계산에 이용될 Diagonal 방향의 타일 인덱스
	* @param TargetIndex - 목표 지점의 타일 인덱스
	*/
	void UpdateOneDiagonalPath(const int32 CurrentIndex, const int32 DiagonalPathIndex, const int32 TargetIndex);

	/**
	* F 비용이 가장 낮은 길의 Index를 얻어냅니다
	* @return F 비용이 가장 낮은 길의 Index
	*/
	int32 FindMinCostFIndex();

	/**
	* 타일 주변에 벽이 있는지 여부를 확인합니다.
	* @param TileIndex - 타일의 Index
	* @param CardinalIndex - 확인할 Cardinal 방향에 있는 타일의 Index
	* @param CoverDirectionArr - 엄폐 방향을 담고있는 배열입니다.
	*/
	void CheckWallAroundOneDirection(const int32 TileIndex, const int CardinalIndex, TArray<FVector>& CoverDirectionArr);

	/**
	* 타일이 이동가능한지 여부를 확인합니다.
	* @param TileIndex - 확인할 타일의 인덱스
	* @return 이동 가능 여부
	*/
	bool CheckAvailability(int32 TileIndex);

	/**
	* 엄폐할수 있는 벽에 엄폐 Mesh를 생성합니다.
	* @param OriginTileIndex - 벽이 있는 타일의 인덱스
	*/
	void MakingCoverNotice(int32 OriginTileIndex);
};
