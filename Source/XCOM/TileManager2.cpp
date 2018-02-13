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

		//���� TileIndexInRange ũ�� ���
	}
	UE_LOG(LogTemp, Warning, L"%d ���� Wall!", TileIndexInRange.Num());

	//UE_LOG(LogTemp, Warning, L"%d", TileIndexInRange.Num());
	//FindPath(OverlappedTileIndex, TileIndexInRange);
	AvailableTiles = FindPath(OverlappedTileIndex, MovingAbility, TileIndexInRange);

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

	for (Path path : PathArr) {
		path.Clear();
	}
	OpenList.Empty();
	ClosedList.Empty();
	//TileIndexInRange.Empty();

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
		PathGuide = PathArr[PathGuide].ParentIndex;

		//UE_LOG(LogTemp, Warning, L"%d", PathArr[PathGuide].ParentIndex)


		while(PathGuide != StartIndex)
		{
			PathGuide = PathArr[PathGuide].ParentIndex;
			PathArr[TargetIndex].OnTheWay.Add(PathGuide);
		}
		//����
		return true;
	}
	else if (OpenList.Num() == 0) 
	{
		//����
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

	//��
	if (CheckWithinBounds(EastIndex) && IsSameLine(CurrentIndex, 0, EastIndex) && !ClosedList.Contains(EastIndex) && TileIndexInRange.Contains(EastIndex)) {
		//Open List �ȿ� �����Ҷ�
		if (OpenList.Contains(EastIndex)) {
			//���� �ʿ��Ҷ�
			if (PathArr[EastIndex].CostG > NewCostG) {
				PathArr[EastIndex].CostG = NewCostG;
				PathArr[EastIndex].CostF = PathArr[EastIndex].CostG + PathArr[EastIndex].CostH;
				PathArr[EastIndex].ParentIndex = CurrentIndex;
			}
		}
		else // OpenList �� �������� �ʾƼ� ���� �߰��ؾ� �ϴ� ���
		{
			PathArr[EastIndex].CostH = ComputeManhattanDistance(EastIndex, TargetIndex);
			PathArr[EastIndex].CostG = NewCostG;
			PathArr[EastIndex].CostF = PathArr[EastIndex].CostG + PathArr[EastIndex].CostH;
			PathArr[EastIndex].ParentIndex = CurrentIndex;

			OpenList.Add(EastIndex);
		}
	}
	
	//��
	if (CheckWithinBounds(WestIndex) && IsSameLine(CurrentIndex, 0, WestIndex) && !ClosedList.Contains(WestIndex)&& TileIndexInRange.Contains(WestIndex)) {
		if (OpenList.Contains(WestIndex)) {
			//���� �ʿ��Ҷ�
			if (PathArr[WestIndex].CostG > NewCostG) {
				PathArr[WestIndex].CostG = NewCostG;
				PathArr[WestIndex].CostF = PathArr[WestIndex].CostG + PathArr[WestIndex].CostH;
				PathArr[WestIndex].ParentIndex = CurrentIndex;
			}
		}
		else // OpenList �� �������� �ʾƼ� ���� �߰��ؾ� �ϴ� ���
		{
			PathArr[WestIndex].CostH = ComputeManhattanDistance(WestIndex, TargetIndex);
			PathArr[WestIndex].CostG = NewCostG;
			PathArr[WestIndex].CostF = PathArr[WestIndex].CostG + PathArr[WestIndex].CostH;
			PathArr[WestIndex].ParentIndex = CurrentIndex;

			OpenList.Add(WestIndex);

		}
	}

	//��
	if (CheckWithinBounds(SouthIndex) && IsSameLine(CurrentIndex, -1, SouthIndex) && !ClosedList.Contains(SouthIndex) && TileIndexInRange.Contains(SouthIndex)) {

		if (OpenList.Contains(SouthIndex)) {
			//���� �ʿ��Ҷ�
			if (PathArr[SouthIndex].CostG > NewCostG) {
				PathArr[SouthIndex].CostG = NewCostG;
				PathArr[SouthIndex].CostF = PathArr[SouthIndex].CostG + PathArr[SouthIndex].CostH;
				PathArr[SouthIndex].ParentIndex = CurrentIndex;
			}
		}
		else // OpenList �� �������� �ʾƼ� ���� �߰��ؾ� �ϴ� ���
		{
			PathArr[SouthIndex].CostH = ComputeManhattanDistance(SouthIndex, TargetIndex);
			PathArr[SouthIndex].CostG = NewCostG;
			PathArr[SouthIndex].CostF = PathArr[SouthIndex].CostG + PathArr[SouthIndex].CostH;
			PathArr[SouthIndex].ParentIndex = CurrentIndex;

			OpenList.Add(SouthIndex);
		}
	}

	//��
	if (CheckWithinBounds(NorthIndex) && IsSameLine(CurrentIndex, 1, NorthIndex) && !ClosedList.Contains(NorthIndex) && TileIndexInRange.Contains(NorthIndex)) {
		

		if (OpenList.Contains(NorthIndex)) {
			//���� �ʿ��Ҷ�
			if (PathArr[NorthIndex].CostG > NewCostG) {
				PathArr[NorthIndex].CostG = NewCostG;
				PathArr[NorthIndex].CostF = PathArr[NorthIndex].CostG + PathArr[NorthIndex].CostH;
				PathArr[NorthIndex].ParentIndex = CurrentIndex;

			}

		}
		else // OpenList �� �������� �ʾƼ� ���� �߰��ؾ� �ϴ� ���
		{
			PathArr[NorthIndex].CostH = ComputeManhattanDistance(NorthIndex, TargetIndex);
			PathArr[NorthIndex].CostG = NewCostG;
			PathArr[NorthIndex].CostF = PathArr[NorthIndex].CostG + PathArr[NorthIndex].CostH;
			PathArr[NorthIndex].ParentIndex = CurrentIndex;

			OpenList.Add(NorthIndex);


		}
	}
	else {
	}


}

void ATileManager2::UpdateDiagonalPath(int32 CurrentIndex, int32 TargetIndex)
{
	int32 NorthEastIndex = CurrentIndex + x + 1;
	int32 NorthWestIndex = CurrentIndex + x - 1;
	int32 SouthEastIndex = CurrentIndex - x + 1;
	int32 SouthWestIndex = CurrentIndex - x - 1;

	int32 NewCostG = PathArr[CurrentIndex].CostG + (int)(TileSize*1.5);

	// 1�� ���� : �ε����� Ÿ�� �������� �����ϴ��� Ȯ��
	// 2�� ���� : �ٸ� �ٿ� ��ġ�ϰ� �ִ� Ÿ���� ����Ű�� �ʴ��� ( �߸��� Ÿ���� ����Ű���� Ȯ��)
	// 3�� ���� : Diagonal Tile �ֺ� ��尡 Wall �� �������� �ʴ��� Ȯ��  ex) �ϵ� - > ��, �� ���� Ÿ���� Wall ���� Ȯ��
	// 4�� ���� : ���� ����
	// 5�� ���� : Ÿ���� ClosedList �� ���� ������ Ȯ��

	//�ϵ�  
	if (CheckWithinBounds(NorthEastIndex) && IsSameLine(CurrentIndex, 1, NorthEastIndex) && TileIndexInRange.Contains(NorthEastIndex - 1) &&
		TileIndexInRange.Contains(NorthEastIndex - x) && !ClosedList.Contains(NorthEastIndex) && !PathArr[NorthEastIndex].bWall)
	{

		//�̹� OpenList �� �����ϴ� Tile�ϰ�� ���� ����
		if (OpenList.Contains(NorthEastIndex))
		{
			// CostG�� �� ���  ������ �ʿ��� ���
			if (PathArr[NorthEastIndex].CostG > NewCostG) {
				PathArr[NorthEastIndex].CostG = PathArr[CurrentIndex].CostG + (int)(TileSize*1.5);
				PathArr[NorthEastIndex].CostF = PathArr[NorthEastIndex].CostG + PathArr[NorthEastIndex].CostH;
				PathArr[NorthEastIndex].ParentIndex = CurrentIndex;
			}
		}
		else // OpenList �� �������� �ʾƼ� ���� �߰��ؾ� �ϴ� ���
		{
			PathArr[NorthEastIndex].CostH = ComputeManhattanDistance(NorthEastIndex, TargetIndex);
			PathArr[NorthEastIndex].CostG = NewCostG;
			PathArr[NorthEastIndex].CostF = PathArr[NorthEastIndex].CostG + PathArr[NorthEastIndex].CostH;
			PathArr[NorthEastIndex].ParentIndex = CurrentIndex;

			OpenList.Add(NorthEastIndex);
		}
	}

	//�ϼ�
	if (CheckWithinBounds(NorthWestIndex) && IsSameLine(CurrentIndex, 1, NorthEastIndex) && TileIndexInRange.Contains(NorthWestIndex + 1) &&
		TileIndexInRange.Contains(NorthWestIndex - x) && !ClosedList.Contains(NorthWestIndex) && !PathArr[NorthWestIndex].bWall)
	{

		//�̹� OpenList �� �����ϴ� Tile�ϰ�� ���� ����
		if (OpenList.Contains(NorthWestIndex))
		{
			// CostG�� �� ���  ������ �ʿ��� ���
			if (PathArr[NorthWestIndex].CostG > NewCostG) {
				PathArr[NorthWestIndex].CostG = PathArr[CurrentIndex].CostG + (int)(TileSize*1.5);
				PathArr[NorthWestIndex].CostF = PathArr[NorthWestIndex].CostG + PathArr[NorthWestIndex].CostH;
				PathArr[NorthWestIndex].ParentIndex = CurrentIndex;
			}
		}
		else // OpenList �� �������� �ʾƼ� ���� �߰��ؾ� �ϴ� ���
		{
			PathArr[NorthWestIndex].CostH = ComputeManhattanDistance(NorthWestIndex, TargetIndex);
			PathArr[NorthWestIndex].CostG = NewCostG;
			PathArr[NorthWestIndex].CostF = PathArr[NorthWestIndex].CostG + PathArr[NorthWestIndex].CostH;
			PathArr[NorthWestIndex].ParentIndex = CurrentIndex;

			OpenList.Add(NorthWestIndex);
		}
	}

	
	//����
	if (CheckWithinBounds(SouthEastIndex) && IsSameLine(CurrentIndex, -1, SouthEastIndex) && TileIndexInRange.Contains(SouthEastIndex - 1) &&
		TileIndexInRange.Contains(NorthEastIndex + x) && !ClosedList.Contains(SouthEastIndex) && !PathArr[SouthEastIndex].bWall)
	{

		if (OpenList.Contains(SouthEastIndex))
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

	//����
	if (CheckWithinBounds(SouthWestIndex) && IsSameLine(CurrentIndex, -1, SouthWestIndex) && TileIndexInRange.Contains(SouthWestIndex + 1) &&
		TileIndexInRange.Contains(SouthWestIndex + x) && !ClosedList.Contains(SouthWestIndex) && !PathArr[SouthWestIndex].bWall)
	{

		//�̹� OpenList �� �����ϴ� Tile�ϰ�� ���� ����
		if (OpenList.Contains(SouthWestIndex))
		{
			// CostG�� �� ���  ������ �ʿ��� ���
			if (PathArr[SouthWestIndex].CostG > NewCostG) {
				PathArr[SouthWestIndex].CostG = PathArr[CurrentIndex].CostG + (int)(TileSize*1.5);
				PathArr[SouthWestIndex].CostF = PathArr[SouthWestIndex].CostG + PathArr[SouthWestIndex].CostH;
				PathArr[SouthWestIndex].ParentIndex = CurrentIndex;
			}
		}
		else // OpenList �� �������� �ʾƼ� ���� �߰��ؾ� �ϴ� ���
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