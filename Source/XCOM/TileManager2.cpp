// Fill out your copyright notice in the Description page of Project Settings.

#include "TileManager2.h"
#include "Classes/Components/InstancedStaticMeshComponent.h"
#include "Classes/Components/ChildActorComponent.h"
#include "Classes/GameFramework/Actor.h"
#include "Path.h"
#include "Tile.h"


ATileManager2::ATileManager2()
{
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(FName("Root"));
	SetRootComponent(Root);
}

void ATileManager2::BeginPlay()
{
	Super::BeginPlay();
	
	for (int i = 0; i < (x*y); i++) {
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
	PathArr[OverlappedTileIndex].bWall = true;

	//UE_LOG(LogTemp, Warning, L"Vector : %s  , Index : %d", *RelativeLocation.ToString(), OverlappedTileIndex);
}


void ATileManager2::FindingWallOnTile(ATile* TileActor) 
{
	TArray<AActor*> OverlappedTile;
	TileActor->GetOverlappingActors(OverlappedTile);

	if (OverlappedTile.Num() != 0) {
		int32 OverlappedTileIndex = ConvertVectorToIndex(TileActor->GetActorLocation());
		PathArr[OverlappedTileIndex].bWall = true;

		UE_LOG(LogTemp, Warning, L"%d Wall On Tile!", OverlappedTileIndex);
	}
}



// @Param Vector : Tile's World Location;
int32 ATileManager2::ConvertVectorToIndex(FVector WorldLocation) 
{
	FVector RelativeLocation = GetActorLocation() - WorldLocation;

	int collum =FMath::Abs(RelativeLocation.X / TileSize);
	int row = FMath::Abs(RelativeLocation.Y / TileSize);
	
	return (row * x) + collum;
}


FVector ATileManager2::ConvertIndexToVector(int32 Index)
{
	int collum = Index % x;
	int row = Index / x;

	return FVector(collum*TileSize * 110, row*TileSize * 110, Root->GetComponentLocation().Z);
}

void ATileManager2::ConvertVectorToCoord(FVector WorldLocation, OUT int& CoordX, OUT int& CoordY) {
	int32 TileIndex =  ConvertVectorToIndex(WorldLocation);
	CoordX = TileIndex % x;
	CoordY = TileIndex / y;
}



void ATileManager2::GetNearbyTiles(ATile* StartingTile, int32 MovingAbility, TArray<ATile*>& AvailableTiles)
{
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
			else if(!IsSameLine(OverlappedTileIndex, i , TargetIndex))   //오른쪽으로 더이상 타일의 진행이 필요없을때
			{	
				continue;
			}
			else if (PathArr[TargetIndex].bWall == true) 
			{
				continue;
			}

			ATile* TargetTile = Cast<ATile>(ChildActors[ChildActors.Num() - TargetIndex - 1]);
			//PathArr[]ComputeManhattanDistance(OverlappedTileIndex, TargetIndex);
			TileIndexInRange.Add(TargetIndex);
			AvailableTiles.Add(TargetTile);
		}	
	}

	//TileIndexInRange
	FindPath(OverlappedTileIndex, TileIndexInRange);
}


bool ATileManager2::IsSameLine(int32 OverlappedTileIndex, int RowNumber, int32 TargetIndex) 
{
	return ((OverlappedTileIndex / x) + RowNumber) == (TargetIndex / x);
}

void ATileManager2::ClearAllTiles() {
	TArray<AActor*> ChildTiles;
	GetAttachedActors(ChildTiles);

	for (AActor* Tile : ChildTiles) {
		UStaticMeshComponent* TileMesh = Cast<UStaticMeshComponent>(Tile->GetRootComponent());
		TileMesh->SetVisibility(false);
	}
}


int32 ATileManager2::ComputeManhattanDistance(int32 StartIndex, int32 TargetIndex) 
{
	return ((FMath::Abs((StartIndex - TargetIndex)) / x) + (FMath::Abs((StartIndex - TargetIndex)) % x)) * TileSize;
}

void ATileManager2::FindPath(int32 StartingIndex, TArray<int32> TileIndexInRange)
{
	for (int32 TargetIndex : TileIndexInRange) {
		int32 CurrentTileIndex = StartingIndex;
		
	}
}

void ATileManager2::UpdatePathInfo(int32 CurrentIndex, int32 StartIndex ,int32 TargetIndex)
{
	UpdateCardinalPath(CurrentIndex, TargetIndex);

	UpdateDiagonalPath(CurrentIndex, TargetIndex);


	//종료조건
	if (OpenList.Find(TargetIndex)) 
	{
		int32 PathGuide= TargetIndex;
		while(PathGuide != StartIndex)
		{
			PathGuide = PathArr[PathGuide].ParentIndex;
			PathArr[TargetIndex].OnTheWay.Add(PathGuide);
		}
		//성공
		return;
	}
	else if (OpenList.Num() == 0) 
	{
		//실패
		return;
	}

	//F비용 가장 낮은걸로 진행
	int NextPathIndex = FindMinCostFIndex();
	OpenList.Remove(NextPathIndex);
	ClosedList.Add(NextPathIndex);

	UpdatePathInfo(NextPathIndex, TargetIndex);
	//CurrentNode 이동..
}

void ATileManager2::UpdateCardinalPath(int32 CurrentIndex, int32 TargetIndex)
{
	int32 EastIndex = CurrentIndex + 1;
	int32 WestIndex = CurrentIndex - 1;
	int32 SouthIndex = CurrentIndex - x;
	int32 NorthIndex = CurrentIndex + x;

	int32 NewCostG = PathArr[CurrentIndex].CostG + TileSize;

	//동
	if (CheckWithinBounds(EastIndex) && IsSameLine(CurrentIndex, 0, EastIndex) && ! ClosedList.Find(EastIndex)) {
		//Open List 안에 존재할때
		if (OpenList.Find(EastIndex)) {
			//갱신 필요할때
			if (PathArr[EastIndex].CostG > NewCostG) {
				PathArr[EastIndex].CostG = NewCostG;
				PathArr[EastIndex].CostF = PathArr[EastIndex].CostG + PathArr[EastIndex].CostH;
				PathArr[EastIndex].ParentIndex = CurrentIndex;
			}
		}
		else // OpenList 에 존재하지 않아서 새로 추가해야 하는 경우
		{
			PathArr[EastIndex].CostH = ComputeManhattanDistance(EastIndex, TargetIndex);
			PathArr[EastIndex].CostG = NewCostG;
			PathArr[EastIndex].CostF = PathArr[EastIndex].CostG + PathArr[EastIndex].CostH;
			PathArr[EastIndex].ParentIndex = CurrentIndex;

			OpenList.Add(EastIndex);
		}
	}
	
	//서
	if (CheckWithinBounds(WestIndex) && IsSameLine(CurrentIndex, 0, WestIndex) && !ClosedList.Find(WestIndex)) {
		if (OpenList.Find(WestIndex)) {
			//갱신 필요할때
			if (PathArr[WestIndex].CostG > NewCostG) {
				PathArr[WestIndex].CostG = NewCostG;
				PathArr[WestIndex].CostF = PathArr[WestIndex].CostG + PathArr[WestIndex].CostH;
				PathArr[WestIndex].ParentIndex = CurrentIndex;
			}
		}
		else // OpenList 에 존재하지 않아서 새로 추가해야 하는 경우
		{
			PathArr[WestIndex].CostH = ComputeManhattanDistance(WestIndex, TargetIndex);
			PathArr[WestIndex].CostG = NewCostG;
			PathArr[WestIndex].CostF = PathArr[WestIndex].CostG + PathArr[WestIndex].CostH;
			PathArr[WestIndex].ParentIndex = CurrentIndex;

			OpenList.Add(WestIndex);
		}
	}

	//남
	if (CheckWithinBounds(SouthIndex) && IsSameLine(CurrentIndex, -1, SouthIndex) && !ClosedList.Find(SouthIndex)) {
		if (OpenList.Find(SouthIndex)) {
			//갱신 필요할때
			if (PathArr[SouthIndex].CostG > NewCostG) {
				PathArr[SouthIndex].CostG = NewCostG;
				PathArr[SouthIndex].CostF = PathArr[SouthIndex].CostG + PathArr[SouthIndex].CostH;
				PathArr[SouthIndex].ParentIndex = CurrentIndex;
			}
		}
		else // OpenList 에 존재하지 않아서 새로 추가해야 하는 경우
		{
			PathArr[SouthIndex].CostH = ComputeManhattanDistance(SouthIndex, TargetIndex);
			PathArr[SouthIndex].CostG = NewCostG;
			PathArr[SouthIndex].CostF = PathArr[SouthIndex].CostG + PathArr[SouthIndex].CostH;
			PathArr[SouthIndex].ParentIndex = CurrentIndex;

			OpenList.Add(SouthIndex);
		}
	}

	//북
	if (CheckWithinBounds(NorthIndex) && IsSameLine(CurrentIndex, 1, NorthIndex) && !ClosedList.Find(NorthIndex)) {
		if (OpenList.Find(NorthIndex)) {
			//갱신 필요할때
			if (PathArr[NorthIndex].CostG > NewCostG) {
				PathArr[NorthIndex].CostG = NewCostG;
				PathArr[NorthIndex].CostF = PathArr[NorthIndex].CostG + PathArr[NorthIndex].CostH;
				PathArr[NorthIndex].ParentIndex = CurrentIndex;
			}
		}
		else // OpenList 에 존재하지 않아서 새로 추가해야 하는 경우
		{
			PathArr[NorthIndex].CostH = ComputeManhattanDistance(NorthIndex, TargetIndex);
			PathArr[NorthIndex].CostG = NewCostG;
			PathArr[NorthIndex].CostF = PathArr[NorthIndex].CostG + PathArr[NorthIndex].CostH;
			PathArr[NorthIndex].ParentIndex = CurrentIndex;

			OpenList.Add(NorthIndex);
		}
	}
}

void ATileManager2::UpdateDiagonalPath(int32 CurrentIndex, int32 TargetIndex)
{
	int32 NorthEastIndex = CurrentIndex + x + 1;
	int32 NorthWestIndex = CurrentIndex + x - 1;
	int32 SouthEastIndex = CurrentIndex - x + 1;
	int32 SouthWestIndex = CurrentIndex - x - 1;

	int32 NewCostG = PathArr[CurrentIndex].CostG + (int)(TileSize*1.5);

	// 1번 조건 : 인덱스가 타일 범위내에 존재하는지 확인
	// 2번 조건 : 다른 줄에 위치하고 있는 타일을 가르키진 않는지 ( 잘못된 타일을 가르키는지 확인)
	// 3번 조건 : Diagonal Tile 주변 노드가 Wall 로 막혀있지 않는지 확인  ex) 북동 - > 북, 동 방향 타일이 Wall 인지 확인
	// 4번 조건 : 위와 동일
	// 5번 조건 : 타일이 ClosedList 에 있지 않은지 확인

	//북동  
	if (CheckWithinBounds(NorthEastIndex) && IsSameLine(CurrentIndex, 1, NorthEastIndex) && TileIndexInRange.Find(NorthEastIndex - 1) &&
		TileIndexInRange.Find(NorthEastIndex - x) && !ClosedList.Find(NorthEastIndex))
	{

		//이미 OpenList 에 존재하는 Tile일경우 갱신 여부
		if (OpenList.Find(NorthEastIndex))
		{
			// CostG의 비교 결과  갱신이 필요한 경우
			if (PathArr[NorthEastIndex].CostG > NewCostG) {
				PathArr[NorthEastIndex].CostG = PathArr[CurrentIndex].CostG + (int)(TileSize*1.5);
				PathArr[NorthEastIndex].CostF = PathArr[NorthEastIndex].CostG + PathArr[NorthEastIndex].CostH;
				PathArr[NorthEastIndex].ParentIndex = CurrentIndex;
			}
		}
		else // OpenList 에 존재하지 않아서 새로 추가해야 하는 경우
		{
			PathArr[NorthEastIndex].CostH = ComputeManhattanDistance(NorthEastIndex, TargetIndex);
			PathArr[NorthEastIndex].CostG = NewCostG;
			PathArr[NorthEastIndex].CostF = PathArr[NorthEastIndex].CostG + PathArr[NorthEastIndex].CostH;
			PathArr[NorthEastIndex].ParentIndex = CurrentIndex;

			OpenList.Add(NorthEastIndex);
		}
	}

	//북서
	if (CheckWithinBounds(NorthWestIndex) && IsSameLine(CurrentIndex, 1, NorthEastIndex) && TileIndexInRange.Find(NorthWestIndex + 1) &&
		TileIndexInRange.Find(NorthWestIndex - x) && !ClosedList.Find(NorthWestIndex))
	{

		//이미 OpenList 에 존재하는 Tile일경우 갱신 여부
		if (OpenList.Find(NorthWestIndex))
		{
			// CostG의 비교 결과  갱신이 필요한 경우
			if (PathArr[NorthWestIndex].CostG > NewCostG) {
				PathArr[NorthWestIndex].CostG = PathArr[CurrentIndex].CostG + (int)(TileSize*1.5);
				PathArr[NorthWestIndex].CostF = PathArr[NorthWestIndex].CostG + PathArr[NorthWestIndex].CostH;
				PathArr[NorthWestIndex].ParentIndex = CurrentIndex;
			}
		}
		else // OpenList 에 존재하지 않아서 새로 추가해야 하는 경우
		{
			PathArr[NorthWestIndex].CostH = ComputeManhattanDistance(NorthWestIndex, TargetIndex);
			PathArr[NorthWestIndex].CostG = NewCostG;
			PathArr[NorthWestIndex].CostF = PathArr[NorthWestIndex].CostG + PathArr[NorthWestIndex].CostH;
			PathArr[NorthWestIndex].ParentIndex = CurrentIndex;

			OpenList.Add(NorthWestIndex);
		}
	}

	
	//남동
	if (CheckWithinBounds(SouthEastIndex) && IsSameLine(CurrentIndex, -1, SouthEastIndex) && TileIndexInRange.Find(SouthEastIndex - 1) &&
		TileIndexInRange.Find(NorthEastIndex + x) && !ClosedList.Find(SouthEastIndex))
	{

		if (OpenList.Find(SouthEastIndex))
		{
			if (PathArr[SouthEastIndex].CostG > NewCostG) {
				PathArr[SouthEastIndex].CostG = PathArr[CurrentIndex].CostG + (int)(TileSize*1.5);
				PathArr[SouthEastIndex].CostF = PathArr[SouthEastIndex].CostG + PathArr[SouthEastIndex].CostH;
				PathArr[SouthEastIndex].ParentIndex = CurrentIndex;
			}
		}
		else 
		{
			PathArr[SouthEastIndex].CostH = ComputeManhattanDistance(SouthEastIndex, TargetIndex);
			PathArr[SouthEastIndex].CostG = NewCostG;
			PathArr[SouthEastIndex].CostF = PathArr[SouthEastIndex].CostG + PathArr[SouthEastIndex].CostH;
			PathArr[SouthEastIndex].ParentIndex = CurrentIndex;

			OpenList.Add(SouthEastIndex);
		}
	}

	//남서
	if (CheckWithinBounds(SouthWestIndex) && IsSameLine(CurrentIndex, -1, SouthWestIndex) && TileIndexInRange.Find(SouthWestIndex + 1) &&
		TileIndexInRange.Find(SouthWestIndex + x) && !ClosedList.Find(SouthWestIndex))
	{

		//이미 OpenList 에 존재하는 Tile일경우 갱신 여부
		if (OpenList.Find(SouthWestIndex))
		{
			// CostG의 비교 결과  갱신이 필요한 경우
			if (PathArr[SouthWestIndex].CostG > NewCostG) {
				PathArr[SouthWestIndex].CostG = PathArr[CurrentIndex].CostG + (int)(TileSize*1.5);
				PathArr[SouthWestIndex].CostF = PathArr[SouthWestIndex].CostG + PathArr[SouthWestIndex].CostH;
				PathArr[SouthWestIndex].ParentIndex = CurrentIndex;
			}
		}
		else // OpenList 에 존재하지 않아서 새로 추가해야 하는 경우
		{
			PathArr[SouthWestIndex].CostH = ComputeManhattanDistance(SouthWestIndex, TargetIndex);
			PathArr[SouthWestIndex].CostG = NewCostG;
			PathArr[SouthWestIndex].CostF = PathArr[SouthWestIndex].CostG + PathArr[SouthWestIndex].CostH;
			PathArr[SouthWestIndex].ParentIndex = CurrentIndex;

			OpenList.Add(SouthWestIndex);
		}
	}
}

bool ATileManager2::CheckWithinBounds(int32 TileIndex)
{
	return  (0 <= TileIndex) && (TileIndex < (x * y));
}

int32 ATileManager2::FindMinCostFIndex() {
	int32 MinCostF = INT_MAX;
	int32 MinCostPathIndex = 0;

	for (int i = OpenList.Num() - 1 ; i >= 0; i--) {
		if (MinCostF > PathArr[OpenList[i]].CostF) {
			MinCostF = PathArr[OpenList[i]].CostF;
			MinCostPathIndex = OpenList[i];
		}
	}

	return MinCostPathIndex;
}