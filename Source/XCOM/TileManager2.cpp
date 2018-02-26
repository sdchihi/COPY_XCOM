// Fill out your copyright notice in the Description page of Project Settings.

#include "TileManager2.h"
#include "Classes/Components/InstancedStaticMeshComponent.h"
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
	
	for (int i = 0; i < (x*y); i++) {
		int collum = i % x;
		int row = i / x;
		
		FTransform instanceTransform = FTransform(FVector(collum*TileSize*110+ 0.1, row*TileSize*110 + 0.1, Root->GetComponentLocation().Z));
		instanceTransform.SetScale3D(FVector(TileSize, TileSize, 1));

		// Tile ����
		ATile* TileActor = GetWorld()->SpawnActor<ATile>(
			TileBlueprint,
			FVector(collum*TileSize + collum*0.001, row*TileSize + row*0.001, Root->GetComponentLocation().Z),
			FRotator(0,0,0)
			);

		if (!TileActor) {
			UE_LOG(LogTemp, Warning, L"Wall BP ���� �ʿ�");
			return;
		}
		TileActor->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
		UStaticMeshComponent* ActorMeshComponent = TileActor->FindComponentByClass<UStaticMeshComponent>();

		// Delegate ����
		ActorMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &ATileManager2::OnOverlapBegin);
		ActorMeshComponent->OnBeginCursorOver.AddDynamic(this, &ATileManager2::MouseOnTile);
		ActorMeshComponent->OnEndCursorOver.AddDynamic(this,& ATileManager2::EndMouseOnTile);


		//Path ������ ��� Array �ʱ�ȭ
		
		PathArr.Add(Path());

		//Path ����
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

	return FVector(collum*TileSize, row*TileSize, Root->GetComponentLocation().Z);
}

void ATileManager2::ConvertVectorToCoord(FVector WorldLocation, OUT int& CoordX, OUT int& CoordY) {
	int32 TileIndex =  ConvertVectorToIndex(WorldLocation);
	CoordX = TileIndex % x;
	CoordY = TileIndex / y;
}



void ATileManager2::GetAvailableTiles(ATile* StartingTile, int32 MovingAbility, TArray<ATile*>& AvailableTiles)
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

			if (TargetIndex > (x*y - 1))  //Ÿ�� ũ�⸦ ����� ����
			{
				continue;
			}
			else if (TargetIndex < 0) {   //Ÿ�� ũ�⸦ ����� ����
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

	UE_LOG(LogTemp, Warning, L"%d ���� Wall!", TileIndexInRange.Num());

	AvailableTiles = FindPath(OverlappedTileIndex, MovingAbility, TileIndexInRange);
}


bool ATileManager2::IsSameLine(int32 OverlappedTileIndex, int RowNumber, int32 TargetIndex) 
{
	return ((OverlappedTileIndex / x) + RowNumber) == (TargetIndex / x);
}

void ATileManager2::ClearAllTiles(bool bClearAll) {
	TArray<AActor*> ChildTiles;
	GetAttachedActors(ChildTiles);

	for (AActor* Tile : ChildTiles) {
		UStaticMeshComponent* TileMesh = Cast<UStaticMeshComponent>(Tile->GetRootComponent());
		TileMesh->SetVisibility(false);
	}

	if (bClearAll) {
		for (Path& path : PathArr) {
			path.Clear(true);
		}
	}
	else {
		for (Path& path : PathArr) {
			path.Clear();
		}
	}

	OpenList.Empty();
	ClosedList.Empty();
}



int32 ATileManager2::ComputeManhattanDistance(int32 StartIndex, int32 TargetIndex) 
{
	return ((FMath::Abs((StartIndex - TargetIndex)) / x) + (FMath::Abs((StartIndex - TargetIndex)) % x)) * TileSize;
}

TArray<ATile*> ATileManager2::FindPath(int32 StartingIndex,int32 MovingAbility,TArray<int32> TileIndexInRange)
{

	TArray<ATile*> AvailableTiles;
	TArray<AActor*> ChildActors;
	GetAttachedActors(ChildActors);

	ClosedList.Add(StartingIndex);
	for (int32 TargetIndex : TileIndexInRange) {
		
		bool bFindPath = UpdatePathInfo(StartingIndex,StartingIndex,TargetIndex);

		if (bFindPath) 
		{
			UE_LOG(LogTemp, Warning, L"%d is Available", TargetIndex);
			int32 PathLenght = PathArr[TargetIndex].OnTheWay.Num();
			for (int i = 0; i < PathLenght; i++) {
				UE_LOG(LogTemp, Warning, L"%d", PathArr[TargetIndex].OnTheWay[i]);
			}
			if (PathLenght <= MovingAbility) {
				AvailableTiles.Add(Cast<ATile>(ChildActors[ChildActors.Num() - TargetIndex - 1]));
			}
		}
		ClearAllTiles();
	}
	return AvailableTiles;
}

bool ATileManager2::UpdatePathInfo(int32 CurrentIndex, int32 StartIndex ,int32 TargetIndex)
{
	UpdateCardinalPath(CurrentIndex, TargetIndex);
	UpdateDiagonalPath(CurrentIndex, TargetIndex);

	//��������
	if (OpenList.Contains(TargetIndex)) 
	{
		int32 PathGuide= TargetIndex;
		PathArr[TargetIndex].OnTheWay.Add(TargetIndex);
		PathGuide = PathArr[PathGuide].ParentIndex;
		while(PathGuide != StartIndex)
		{
			PathArr[TargetIndex].OnTheWay.Add(PathGuide);
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
		//F��� ���� �����ɷ� ����

		int NextPathIndex = FindMinCostFIndex();
		OpenList.Remove(NextPathIndex);
		ClosedList.Add(NextPathIndex);

		return UpdatePathInfo(NextPathIndex, StartIndex, TargetIndex);
	}
}

void ATileManager2::UpdateCardinalPath(int32 CurrentIndex, int32 TargetIndex)
{
	int32 EastIndex = CurrentIndex + 1;
	int32 WestIndex = CurrentIndex - 1;
	int32 SouthIndex = CurrentIndex - x;
	int32 NorthIndex = CurrentIndex + x;

	int32 NewCostG = PathArr[CurrentIndex].CostG + TileSize;

	UpdateOneCardinalPath(CurrentIndex, EastIndex, TargetIndex);
	UpdateOneCardinalPath(CurrentIndex, WestIndex, TargetIndex);
	UpdateOneCardinalPath(CurrentIndex, SouthIndex, TargetIndex);
	UpdateOneCardinalPath(CurrentIndex, NorthIndex, TargetIndex);
}

void ATileManager2::UpdateOneCardinalPath(int32 CurrentIndex, int32 CardinalPathIndex, int32 TargetIndex) {
	int32 RowNumber = 0;
	int32 NewCostG = PathArr[CurrentIndex].CostG + TileSize;

	if (CardinalPathIndex == (CurrentIndex + x)) {
		RowNumber = 1;
	}
	else if (CardinalPathIndex == (CurrentIndex - x)) {
		RowNumber = -1;
	}
	else {
		RowNumber = 0;
	}
	
	if (CheckWithinBounds(CardinalPathIndex) && IsSameLine(CurrentIndex, RowNumber, CardinalPathIndex) && !ClosedList.Contains(CardinalPathIndex) && TileIndexInRange.Contains(CardinalPathIndex)) {
		//Open List �ȿ� �����Ҷ�
		if (OpenList.Contains(CardinalPathIndex)) {
			//���� �ʿ��Ҷ�
			if (PathArr[CardinalPathIndex].CostG > NewCostG) {
				PathArr[CardinalPathIndex].CostG = NewCostG;
				PathArr[CardinalPathIndex].CostF = PathArr[CardinalPathIndex].CostG + PathArr[CardinalPathIndex].CostH;
				PathArr[CardinalPathIndex].ParentIndex = CurrentIndex;
			}
		}
		else // OpenList �� �������� �ʾƼ� ���� �߰��ؾ� �ϴ� ���
		{
			PathArr[CardinalPathIndex].CostH = ComputeManhattanDistance(CardinalPathIndex, TargetIndex);
			PathArr[CardinalPathIndex].CostG = NewCostG;
			PathArr[CardinalPathIndex].CostF = PathArr[CardinalPathIndex].CostG + PathArr[CardinalPathIndex].CostH;
			PathArr[CardinalPathIndex].ParentIndex = CurrentIndex;

			OpenList.Add(CardinalPathIndex);
		}
	}
}

void ATileManager2::UpdateDiagonalPath(int32 CurrentIndex, int32 TargetIndex)
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

void ATileManager2::UpdateOneDiagonalPath(int32 CurrentIndex, int32 DiagonalPathIndex, int32 TargetIndex) {
	
	int32 NorthEastIndex = CurrentIndex + x + 1;
	int32 NorthWestIndex = CurrentIndex + x - 1;
	int32 SouthEastIndex = CurrentIndex - x + 1;
	int32 SouthWestIndex = CurrentIndex - x - 1;

	int32 FromCurrentToDiagonal = 0;
	int32 RowDifference = 0;
	int32 CollumDifference = 0;

	int32 NewCostG = PathArr[CurrentIndex].CostG + (int)(TileSize*1.5);

	if (DiagonalPathIndex == NorthEastIndex) {
		FromCurrentToDiagonal = 1;
		RowDifference = -x;
		CollumDifference = -1;
	}
	else if (DiagonalPathIndex == NorthWestIndex) {
		FromCurrentToDiagonal = 1;
		RowDifference = -x;
		CollumDifference = 1;
	}
	else if (DiagonalPathIndex == SouthEastIndex) {
		FromCurrentToDiagonal = -1;
		RowDifference = x;
		CollumDifference = -1;
	}
	else if (DiagonalPathIndex == SouthWestIndex) {
		FromCurrentToDiagonal = -1;
		RowDifference = x;
		CollumDifference = 1;
	}
	else {
		UE_LOG(LogTemp, Warning, L"�߸��� Diagonal Index");
		return;
	}


	// 1�� ���� : �ε����� Ÿ�� �������� �����ϴ��� Ȯ��
	// 2�� ���� : �ٸ� �ٿ� ��ġ�ϰ� �ִ� Ÿ���� ����Ű�� �ʴ��� ( �߸��� Ÿ���� ����Ű���� Ȯ��)
	// 3�� ���� : Diagonal Tile �ֺ� ��尡 Wall �� �������� �ʴ��� Ȯ��  ex) �ϵ� - > ��, �� ���� Ÿ���� Wall ���� Ȯ��
	// 4�� ���� : ���� ����
	// 5�� ���� : Ÿ���� ClosedList �� ���� ������ Ȯ��

	if (CheckWithinBounds(DiagonalPathIndex) && IsSameLine(CurrentIndex, FromCurrentToDiagonal, DiagonalPathIndex) && TileIndexInRange.Contains(DiagonalPathIndex + RowDifference) &&
		TileIndexInRange.Contains(DiagonalPathIndex + CollumDifference) && !ClosedList.Contains(DiagonalPathIndex) && !PathArr[DiagonalPathIndex].bWall)
	{

		//�̹� OpenList �� �����ϴ� Tile�ϰ�� ���� ����
		if (OpenList.Contains(DiagonalPathIndex))
		{
			// CostG�� �� ���  ������ �ʿ��� ���
			if (PathArr[DiagonalPathIndex].CostG > NewCostG) {
				PathArr[DiagonalPathIndex].CostG = PathArr[CurrentIndex].CostG + (int)(TileSize*1.5);
				PathArr[DiagonalPathIndex].CostF = PathArr[DiagonalPathIndex].CostG + PathArr[DiagonalPathIndex].CostH;
				PathArr[DiagonalPathIndex].ParentIndex = CurrentIndex;
			}
		}
		else // OpenList �� �������� �ʾƼ� ���� �߰��ؾ� �ϴ� ���
		{
			PathArr[DiagonalPathIndex].CostH = ComputeManhattanDistance(DiagonalPathIndex, TargetIndex);
			PathArr[DiagonalPathIndex].CostG = NewCostG;
			PathArr[DiagonalPathIndex].CostF = PathArr[DiagonalPathIndex].CostG + PathArr[DiagonalPathIndex].CostH;
			PathArr[DiagonalPathIndex].ParentIndex = CurrentIndex;

			OpenList.Add(DiagonalPathIndex);
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


void ATileManager2::SetDecalVisibilityOnTile(TArray<int32> PathIndices,int32 NumberOfTimes, bool bVisibility) {
	if (NumberOfTimes == 0) { return; }

	TArray<AActor*> ChildActors;
	GetAttachedActors(ChildActors);

	int32 TargetTileIndex = PathIndices[NumberOfTimes-1];

	ATile* Tile = Cast<ATile>(ChildActors[ChildActors.Num() - TargetTileIndex - 1]);
	Tile->SetDecalVisibility(bVisibility);
	SetDecalVisibilityOnTile(PathIndices, NumberOfTimes-1 , bVisibility);
}

void ATileManager2::MouseOnTile(UPrimitiveComponent* OverlappedComponent) {
	FVector ActorLocation = OverlappedComponent->GetOwner()->GetActorLocation();
	int32 TileIndex = ConvertVectorToIndex(ActorLocation);

	SetDecalVisibilityOnTile(PathArr[TileIndex].OnTheWay, PathArr[TileIndex].OnTheWay.Num(), true);
};

void ATileManager2::EndMouseOnTile(UPrimitiveComponent* OverlappedComponent) {
	FVector ActorLocation = OverlappedComponent->GetOwner()->GetActorLocation();
	int32 TileIndex = ConvertVectorToIndex(ActorLocation);

	SetDecalVisibilityOnTile(PathArr[TileIndex].OnTheWay, PathArr[TileIndex].OnTheWay.Num(), false);
};