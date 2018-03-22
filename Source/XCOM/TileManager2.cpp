// Fill out your copyright notice in the Description page of Project Settings.

#include "TileManager2.h"
#include "Classes/Components/InstancedStaticMeshComponent.h"
#include "Classes/Components/DecalComponent.h"
#include "Classes/Components/ChildActorComponent.h"
#include "Classes/GameFramework/Actor.h"
#include "Path.h"
#include "Tile.h"

ATileManager2::ATileManager2()
{
	PrimaryActorTick.bCanEverTick = false;
	Root = CreateDefaultSubobject<USceneComponent>(FName("Root"));


	Root->SetMobility(EComponentMobility::Static);
	SetRootComponent(Root);
}

void ATileManager2::BeginPlay()
{
	Super::BeginPlay();
	
	for (int i = 0; i < (x*y); i++) 
	{
		int collum = i % x;
		int row = i / x;
		
		FTransform instanceTransform = FTransform(FVector(collum*TileSize*110+ 0.1, row*TileSize*110 + 0.1, Root->GetComponentLocation().Z));
		instanceTransform.SetScale3D(FVector(TileSize, TileSize, 1));

		// Tile 생성
		ATile* TileActor = GetWorld()->SpawnActor<ATile>(
			TileBlueprint,
			FVector(collum*TileSize + collum*0.001, row*TileSize + row*0.001, Root->GetComponentLocation().Z),
			FRotator(0,0,0)
			);

		if (!TileActor) {
			UE_LOG(LogTemp, Warning, L"Wall BP 지정 필요");
			return;
		}
		TileActor->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
		UStaticMeshComponent* ActorMeshComponent = TileActor->FindComponentByClass<UStaticMeshComponent>();

		// Delegate 지정
		ActorMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &ATileManager2::OnOverlapBegin);
		ActorMeshComponent->OnBeginCursorOver.AddDynamic(this, &ATileManager2::MouseOnTile);
		ActorMeshComponent->OnEndCursorOver.AddDynamic(this,& ATileManager2::EndMouseOnTile);

		//Path 정보를 담는 Array 초기화
		
		PathArr.Add(Path());

		//Path 갱신
		FindingWallOnTile(TileActor);
	}
}

void ATileManager2::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void ATileManager2::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	int32 OverlappedTileIndex = ConvertVectorToIndex(SweepResult.Actor->GetActorLocation());
	//Todo 
	//PathArr[OverlappedTileIndex].bWall = false;
}

/**
* 타일위에 Wall이 있는지 확인.. 
* TODO
*/
void ATileManager2::FindingWallOnTile(ATile* TileActor) 
{
	TArray<AActor*> OverlappedTile;
	TileActor->GetOverlappingActors(OverlappedTile);

	if (OverlappedTile.Num() != 0) 
	{
		int32 OverlappedTileIndex = ConvertVectorToIndex(TileActor->GetActorLocation());
		PathArr[OverlappedTileIndex].bWall = true;

		//UE_LOG(LogTemp, Warning, L"%d Wall On Tile!", OverlappedTileIndex);
	}
}

/**
* Location을 이용해 Tile의 Index를 얻어냅니다.
* @param WorldLocation - 타일의 월드 좌표
* @return 타일의 Index를 반환합니다.
*/
int32 ATileManager2::ConvertVectorToIndex(const FVector WorldLocation) 
{
	FVector RelativeLocation = GetActorLocation() - WorldLocation;

	int collum =FMath::Abs(RelativeLocation.X / TileSize);
	int row = FMath::Abs(RelativeLocation.Y / TileSize);
	
	return (row * x) + collum;
}

/**
* 타일의 Index를 이용해 월드 좌표를 얻어냅니다.
* @param Index - 타일의 인덱스
* @return 타일의 월드좌표를 반환합니다.
*/
FVector ATileManager2::ConvertIndexToVector(const int32 Index)
{
	int collum = Index % x;
	int row = Index / x;

	return FVector(collum*TileSize, row*TileSize, Root->GetComponentLocation().Z);
}

void ATileManager2::ConvertVectorToCoord(FVector WorldLocation, OUT int& CoordX, OUT int& CoordY) {
	int32 TileIndex =  ConvertVectorToIndex(WorldLocation);
	CoordX = TileIndex % x;
	CoordY = TileIndex / y;
}


/**
* 한 타일을 기준으로 이동 가능한 타일들을 얻어냅니다.
* @param StartingTile - 시작점이될 Pawn이 올라가있는 타일
* @param MovingAbility - 행동력 ( 최대 이동 거리 )
* @param AvailableTiles - 이동 가능한 타일들이 담길 Array
*/
void ATileManager2::GetAvailableTiles(ATile* StartingTile, const int32 MovingAbility, int32 MovableStepsPerAct, TArray<ATile*>& AvailableTiles)
{
	TileIndexInRange.Empty();
	int OverlappedTileIndex = ConvertVectorToIndex(StartingTile->GetActorLocation());
	
	TArray<AActor*> ChildActors;
	GetAttachedActors(ChildActors);
	ChildActors[OverlappedTileIndex];

	int CurrentStep = MovingAbility;
	for (int32 i = MovingAbility; i >= -MovingAbility; i--) 
	{
		int32 CurrentStep = MovingAbility - FMath::Abs(i);
		int32 TargetIndex;

		for (int32 j = CurrentStep; j >= -CurrentStep; j--) 
		{
			TargetIndex = OverlappedTileIndex + (i * x) + j;

			if (TargetIndex > (x*y - 1))  //타일 크기를 벗어날때 제외
			{
				continue;
			}
			else if (TargetIndex < 0) {   //타일 크기를 벗어날때 제외
				continue;
			}
			else if(!IsSameLine(OverlappedTileIndex, i , TargetIndex)) 
			{	
				continue;
			}
			else if (PathArr[TargetIndex].bWall == true) 
			{
				continue;
			}
			ATile* TargetTile = Cast<ATile>(ChildActors[ChildActors.Num() - TargetIndex - 1]);
			TileIndexInRange.Add(TargetIndex);
		}	
	}
	AvailableTiles = FindPath(OverlappedTileIndex, MovingAbility, MovableStepsPerAct, TileIndexInRange);
}

/**
* 
* @param OverlappedTileIndex - 타일의 OverlappedTileIndex 좌표
* @param RowNumber - 타일의 월드 좌표
* @param TargetIndex - 타일의 월드 좌표
* @return 타일의 Index를 반환합니다.
*/
bool ATileManager2::IsSameLine(const int32 OverlappedTileIndex, const int RowNumber, const int32 TargetIndex)
{
	return ((OverlappedTileIndex / x) + RowNumber) == (TargetIndex / x);
}

/**
* A* 알고리즘에 이용되는 데이터들을 초기화한다.
* @param bClearAll - Path정보까지 모두 지울지 결정하는 변수
*/
void ATileManager2::ClearAllTiles(const bool bClearAll) {
	TArray<AActor*> ChildTiles;
	GetAttachedActors(ChildTiles);

	

	if (bClearAll) 
	{
		for (Path& path : PathArr) 
		{
			path.Clear(true);
		}

		for (AActor* Tile : ChildTiles)
		{
			UStaticMeshComponent* TileMesh = Cast<UStaticMeshComponent>(Tile->GetRootComponent());
			UDecalComponent* Decal = Tile->FindComponentByClass<UDecalComponent>();
			TileMesh->SetVisibility(false);
			Decal->SetVisibility(false);
		}
	}
	else 
	{
		for (Path& path : PathArr) 
		{
			path.Clear();
		}
	}

	OpenList.Empty();
	ClosedList.Empty();
}

/**
* 타일간 맨하탄 거리를 얻어냅니다.
* @param StartIndex - 시작 타일 인덱스
* @param TargetIndex - 목표 타일 인덱스
* @return 맨하탄 거리를 반환합니다.
*/
int32 ATileManager2::ComputeManhattanDistance(const int32 StartIndex, const int32 TargetIndex)
{
	int32 CollumDifference = FMath::Abs((StartIndex / x) - (TargetIndex / x));
	int32 RowDifference = FMath::Abs((StartIndex % x) - (TargetIndex % x));

	return (CollumDifference + RowDifference) * TileSize;
}

/**
* 이동 범위내에 있는 각 타일들에 대해서 A* 알고리즘을 적용해 실제 이동가능한 타일들의 목록을 얻어냅니다.
* @param StartingIndex - 시작점이될 Pawn이 올라가있는 타일
* @param MovingAbility - 행동력 ( 최대 이동 거리 )
* @param TileIndexInRange - 이동 범위내에 존재하는 타일들
* @return 실제 이동가능한 타일들의 Array
*/
TArray<ATile*> ATileManager2::FindPath(const int32 StartingIndex, const int32 MovingAbility, int32 MovableStepsPerAct, TArray<int32> TileIndexInRange)
{
	TArray<ATile*> AvailableTiles;
	TArray<AActor*> ChildActors;
	GetAttachedActors(ChildActors);

	ClosedList.Add(StartingIndex);
	for (int32 TargetIndex : TileIndexInRange)
	{
		
		bool bFindPath = UpdatePathInfo(StartingIndex,StartingIndex,TargetIndex);

		if (bFindPath) 
		{
			int32 PathLength = PathArr[TargetIndex].OnTheWay.Num();
			/*for (int i = 0; i < PathLenght; i++) {
				UE_LOG(LogTemp, Warning, L"%d", PathArr[TargetIndex].OnTheWay[i]);
			}*/
			ATile* TargetTile = Cast<ATile>(ChildActors[ChildActors.Num() - TargetIndex - 1]);
			if (MovableStepsPerAct < PathLength && PathLength <= MovingAbility)
			{
				TargetTile->bCanMoveWithOneAct = false;
				AvailableTiles.Add(TargetTile);
			}
			else if (PathLength <= MovableStepsPerAct)
			{
				TargetTile->bCanMoveWithOneAct = true;
				AvailableTiles.Add(TargetTile);
			}
		}
		ClearAllTiles();
	}
	return AvailableTiles;
}

/**
* A* 알고리즘에 따라 길을 찾습니다.
* @param CurrentIndex - 길찾기 진행중인 타일 인덱스 
* @param StartIndex - 시작 지점의 타일 인덱스
* @param TargetIndex - 목표 지점의 타일 인덱스
* @return 목표 지점의 타일로 이동가능 여부를 반환합니다.
*/
bool ATileManager2::UpdatePathInfo(const int32 CurrentIndex, const int32 StartIndex , const int32 TargetIndex)
{
	UpdateCardinalPath(CurrentIndex, TargetIndex);
	UpdateDiagonalPath(CurrentIndex, TargetIndex);

	//종료조건
	if (OpenList.Contains(TargetIndex)) 
	{
		int32 PathGuide= TargetIndex;
		PathArr[TargetIndex].OnTheWay.Add(TargetIndex);
		PathArr[TargetIndex].OnTheWayMap.Add(TargetIndex, PathArr[TargetIndex].PathDirection);

		PathGuide = PathArr[PathGuide].ParentIndex;
		while(PathGuide != StartIndex)
		{
			PathArr[TargetIndex].OnTheWay.Add(PathGuide);
			PathArr[TargetIndex].OnTheWayMap.Add(PathGuide, PathArr[PathGuide].PathDirection);

			PathGuide = PathArr[PathGuide].ParentIndex;
		}
		return true;
	}
	else if (OpenList.Num() == 0) 
	{
		return false;
	}
	else 
	{
		//F비용 가장 낮은걸로 진행
		int NextPathIndex = FindMinCostFIndex();
		OpenList.Remove(NextPathIndex);
		ClosedList.Add(NextPathIndex);

		return UpdatePathInfo(NextPathIndex, StartIndex, TargetIndex);
	}
}

/**
* 모든 Cardinal 방향에 대해서 A*알고리즘을 적용합니다.
* @param CurrentIndex - 길찾기 진행중인 타일 인덱스 
* @param TargetIndex - 목표 지점의 타일 인덱스
*/
void ATileManager2::UpdateCardinalPath(const int32 CurrentIndex, const int32 TargetIndex)
{
	int32 EastIndex = CurrentIndex + 1;
	int32 WestIndex = CurrentIndex - 1;
	int32 SouthIndex = CurrentIndex - x;
	int32 NorthIndex = CurrentIndex + x;


	UpdateOneCardinalPath(CurrentIndex, EastIndex, TargetIndex);
	UpdateOneCardinalPath(CurrentIndex, WestIndex, TargetIndex);
	UpdateOneCardinalPath(CurrentIndex, SouthIndex, TargetIndex);
	UpdateOneCardinalPath(CurrentIndex, NorthIndex, TargetIndex);
}

/**
* 하나의 Cadinal 방향의 Tile에 대해서 비용 계산을 합니다.
* @param CurrentIndex - 길찾기 진행중인 타일 인덱스
* @param CardinalPathIndex - 비용계산에 이용될 Cardinal 방향의 타일 인덱스
* @param TargetIndex - 목표 지점의 타일 인덱스
*/
void ATileManager2::UpdateOneCardinalPath(const int32 CurrentIndex, const int32 CardinalPathIndex, const int32 TargetIndex) {
	int32 RowNumber = 0;
	int32 NewCostG = PathArr[CurrentIndex].CostG + TileSize;
	float PathDecalDirection = 0;

	if (CardinalPathIndex == (CurrentIndex + x)) 
	{
		RowNumber = 1;
		PathDecalDirection = 270;
	}
	else if (CardinalPathIndex == (CurrentIndex - x)) 
	{
		RowNumber = -1;
		PathDecalDirection = 90;
	}
	else 
	{
		RowNumber = 0;

		if (CardinalPathIndex == (CurrentIndex + 1)) 
		{
			PathDecalDirection = 180;
		}
		else if(CardinalPathIndex == (CurrentIndex -1)) 
		{
			PathDecalDirection = 0;
		}
	}
	
	if (CheckWithinBounds(CardinalPathIndex) && IsSameLine(CurrentIndex, RowNumber, CardinalPathIndex) && !ClosedList.Contains(CardinalPathIndex) && TileIndexInRange.Contains(CardinalPathIndex)) 
	{
		//Open List 안에 존재할때
		if (OpenList.Contains(CardinalPathIndex)) 
		{
			//갱신 필요할때
			if (PathArr[CardinalPathIndex].CostG > NewCostG) 
			{
				PathArr[CardinalPathIndex].CostG = NewCostG;
				PathArr[CardinalPathIndex].CostF = PathArr[CardinalPathIndex].CostG + PathArr[CardinalPathIndex].CostH;
				PathArr[CardinalPathIndex].ParentIndex = CurrentIndex;
				PathArr[CardinalPathIndex].PathDirection = PathDecalDirection;
			}
		}
		else // OpenList 에 존재하지 않아서 새로 추가해야 하는 경우
		{
			PathArr[CardinalPathIndex].CostH = ComputeManhattanDistance(CardinalPathIndex, TargetIndex);
			PathArr[CardinalPathIndex].CostG = NewCostG;
			PathArr[CardinalPathIndex].CostF = PathArr[CardinalPathIndex].CostG + PathArr[CardinalPathIndex].CostH;
			PathArr[CardinalPathIndex].ParentIndex = CurrentIndex;
			PathArr[CardinalPathIndex].PathDirection = PathDecalDirection;
			OpenList.Add(CardinalPathIndex);
		}
	}
}

/**
* 모든 Diagonal 방향에 대해서 A*알고리즘을 적용합니다.
* @param CurrentIndex - 길찾기 진행중인 타일 인덱스
* @param TargetIndex - 목표 지점의 타일 인덱스
*/
void ATileManager2::UpdateDiagonalPath(const int32 CurrentIndex, const int32 TargetIndex)
{
	int32 NorthEastIndex = CurrentIndex + x + 1;
	int32 NorthWestIndex = CurrentIndex + x - 1;
	int32 SouthEastIndex = CurrentIndex - x + 1;
	int32 SouthWestIndex = CurrentIndex - x - 1;

	UpdateOneDiagonalPath(CurrentIndex, NorthEastIndex, TargetIndex);
	UpdateOneDiagonalPath(CurrentIndex, NorthWestIndex, TargetIndex);
	UpdateOneDiagonalPath(CurrentIndex, SouthEastIndex, TargetIndex);
	UpdateOneDiagonalPath(CurrentIndex, SouthWestIndex, TargetIndex);
}

/**
* 하나의 Diagonal 방향의 Tile에 대해서 비용 계산을 합니다.
* @param CurrentIndex - 길찾기 진행중인 타일 인덱스
* @param DiagonalPathIndex - 비용계산에 이용될 Diagonal 방향의 타일 인덱스
* @param TargetIndex - 목표 지점의 타일 인덱스
*/
void ATileManager2::UpdateOneDiagonalPath(const int32 CurrentIndex, const int32 DiagonalPathIndex, const int32 TargetIndex) {
	
	int32 NorthEastIndex = CurrentIndex + x + 1;
	int32 NorthWestIndex = CurrentIndex + x - 1;
	int32 SouthEastIndex = CurrentIndex - x + 1;
	int32 SouthWestIndex = CurrentIndex - x - 1;

	float PathDecalDirection = 0;

	int32 FromCurrentToDiagonal = 0;
	int32 RowDifference = 0;
	int32 CollumDifference = 0;

	int32 NewCostG = PathArr[CurrentIndex].CostG + (int)(TileSize*1.5);

	if (DiagonalPathIndex == NorthEastIndex) 
	{
		FromCurrentToDiagonal = 1;
		RowDifference = -x;
		CollumDifference = -1;
		PathDecalDirection = 225;
	}
	else if (DiagonalPathIndex == NorthWestIndex) 
	{
		FromCurrentToDiagonal = 1;
		RowDifference = -x;
		CollumDifference = 1;
		PathDecalDirection = 315;
	}
	else if (DiagonalPathIndex == SouthEastIndex) 
	{
		FromCurrentToDiagonal = -1;
		RowDifference = x;
		CollumDifference = -1;
		PathDecalDirection = 135;
	}
	else if (DiagonalPathIndex == SouthWestIndex) 
	{
		FromCurrentToDiagonal = -1;
		RowDifference = x;
		CollumDifference = 1;
		PathDecalDirection = 45;
	}
	else {
		UE_LOG(LogTemp, Warning, L"잘못된 Diagonal Index");
		return;
	}


	// 1번 조건 : 인덱스가 타일 범위내에 존재하는지 확인
	// 2번 조건 : 다른 줄에 위치하고 있는 타일을 가르키진 않는지 ( 잘못된 타일을 가르키는지 확인)
	// 3번 조건 : Diagonal Tile 주변 노드가 Wall 로 막혀있지 않는지 확인  ex) 북동 - > 북, 동 방향 타일이 Wall 인지 확인
	// 4번 조건 : 위와 동일
	// 5번 조건 : 타일이 ClosedList 에 있지 않은지 확인

	if (CheckWithinBounds(DiagonalPathIndex) && IsSameLine(CurrentIndex, FromCurrentToDiagonal, DiagonalPathIndex) && TileIndexInRange.Contains(DiagonalPathIndex + RowDifference) &&
		TileIndexInRange.Contains(DiagonalPathIndex + CollumDifference) && !ClosedList.Contains(DiagonalPathIndex) && !PathArr[DiagonalPathIndex].bWall)
	{

		//이미 OpenList 에 존재하는 Tile일경우 갱신 여부
		if (OpenList.Contains(DiagonalPathIndex))
		{
			// CostG의 비교 결과  갱신이 필요한 경우
			if (PathArr[DiagonalPathIndex].CostG > NewCostG)
			{
				PathArr[DiagonalPathIndex].CostG = PathArr[CurrentIndex].CostG + (int)(TileSize*1.5);
				PathArr[DiagonalPathIndex].CostF = PathArr[DiagonalPathIndex].CostG + PathArr[DiagonalPathIndex].CostH;
				PathArr[DiagonalPathIndex].ParentIndex = CurrentIndex;
				PathArr[DiagonalPathIndex].PathDirection = PathDecalDirection;
			}
		}
		else // OpenList 에 존재하지 않아서 새로 추가해야 하는 경우
		{
			PathArr[DiagonalPathIndex].CostH = ComputeManhattanDistance(DiagonalPathIndex, TargetIndex);
			PathArr[DiagonalPathIndex].CostG = NewCostG;
			PathArr[DiagonalPathIndex].CostF = PathArr[DiagonalPathIndex].CostG + PathArr[DiagonalPathIndex].CostH;
			PathArr[DiagonalPathIndex].ParentIndex = CurrentIndex;
			PathArr[DiagonalPathIndex].PathDirection = PathDecalDirection;

			OpenList.Add(DiagonalPathIndex);
		}
	}
}

/**
* 타일의 인덱스가 최대 범위를 넘어가는지 확인합니다.
* @param TileIndex - 검사하게 될 타일의 index
* @return 해당 타일이 유효 범위를 벗어나는지 여부를 반환합니다.
*/
bool ATileManager2::CheckWithinBounds(const int32 TileIndex)
{
	return  (0 <= TileIndex) && (TileIndex < (x * y));
}

/**
* F 비용이 가장 낮은 길의 Index를 얻어냅니다
* @return F 비용이 가장 낮은 길의 Index
*/
int32 ATileManager2::FindMinCostFIndex() {
	int32 MinCostF = INT_MAX;
	int32 MinCostPathIndex = 0;

	for (int i = OpenList.Num() - 1 ; i >= 0; i--) 
	{
		if (MinCostF > PathArr[OpenList[i]].CostF) 
		{
			MinCostF = PathArr[OpenList[i]].CostF;
			MinCostPathIndex = OpenList[i];
		}
	}

	return MinCostPathIndex;
}

/**
* 타일위에 나타나는 화살표 Decal 방향을 변경하고, 가시성을 제어합니다.
* @param PathInfo - Key : 타일(key)에서 다음 타일로  이동방향(value)을 갖고있는 Map
* @param NumberOfTimes - 함수 반복 횟수
* @param bVisibility - Decal을 보일지 말지 결정합니다.
*/
void ATileManager2::SetDecalVisibilityOnTile(TMap<int32, float> PathInfo, const int32 NumberOfTimes, const bool bVisibility) {
	TArray<AActor*> ChildActors;
	GetAttachedActors(ChildActors);

	for (auto OnePathInfo : PathInfo) 
	{
		int32 TargetTileIndex = OnePathInfo.Key;
		//UE_LOG(LogTemp, Warning, L"Decal Tile Index : %d , Yaw : %f", TargetTileIndex, OnePathInfo.Value);

		ATile* Tile = Cast<ATile>(ChildActors[ChildActors.Num() - TargetTileIndex - 1]);
		Tile->SetDecalVisibility(bVisibility);
		if (bVisibility) {
			Tile->SetDecalRotationYaw(OnePathInfo.Value);
		}
	}
}


void ATileManager2::MouseOnTile(UPrimitiveComponent* OverlappedComponent) {
	FVector ActorLocation = OverlappedComponent->GetOwner()->GetActorLocation();
	int32 TileIndex = ConvertVectorToIndex(ActorLocation);

	SetDecalVisibilityOnTile(PathArr[TileIndex].OnTheWayMap, PathArr[TileIndex].OnTheWay.Num(), true);
};

void ATileManager2::EndMouseOnTile(UPrimitiveComponent* OverlappedComponent) {
	FVector ActorLocation = OverlappedComponent->GetOwner()->GetActorLocation();
	int32 TileIndex = ConvertVectorToIndex(ActorLocation);

	SetDecalVisibilityOnTile(PathArr[TileIndex].OnTheWayMap, PathArr[TileIndex].OnTheWay.Num(), false);
};