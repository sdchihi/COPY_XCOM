// Fill out your copyright notice in the Description page of Project Settings.

#include "TileManager.h"
#include "Classes/Components/InstancedStaticMeshComponent.h"
#include "Classes/Components/DecalComponent.h"
#include "Classes/Components/ChildActorComponent.h"
#include "Classes/GameFramework/Actor.h"
#include "Path.h"
#include "Tile.h"
#include "DestructibleWall.h"
#include "CoveringChecker.h"
#include "PathIndicator.h"

ATileManager::ATileManager()
{
	PrimaryActorTick.bCanEverTick = false;
	Root = CreateDefaultSubobject<USceneComponent>(FName("Root"));


	Root->SetMobility(EComponentMobility::Static);
	SetRootComponent(Root);
}

void ATileManager::BeginPlay()
{
	Super::BeginPlay();
	

	if (CoveringCheckerBlueprint)
	{
		CoveringChecker = GetWorld()->SpawnActor<ACoveringChecker>(
			CoveringCheckerBlueprint,
			FVector(0, 0, 0),
			FRotator(0, 0, 0)
			);
	}

	if (PathIndicatorBlueprint)
	{
		PathIndicator = GetWorld()->SpawnActor<APathIndicator>(
			PathIndicatorBlueprint,
			FVector(0, 0, 0),
			FRotator(0, 0, 0)
			);
	}

	for (int i = 0; i < (x*y); i++) 
	{
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
		ActorMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &ATileManager::OnOverlapBegin);
		ActorMeshComponent->OnComponentEndOverlap.AddDynamic(this, &ATileManager::EndTileOverlap);
		ActorMeshComponent->OnBeginCursorOver.AddDynamic(this, &ATileManager::MouseOnTile);
		ActorMeshComponent->OnEndCursorOver.AddDynamic(this,& ATileManager::EndMouseOnTile);
		//Path ������ ��� Array �ʱ�ȭ
		PathArr.Add(Path());

		//Path ����
		FindingWallOnTile(TileActor);
	}
}

void ATileManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void ATileManager::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FVector TileLocation = OverlappedComp->GetOwner()->GetActorLocation();
	int32 OverlappedTileIndex = ConvertVectorToIndex(TileLocation);  
	APawn* OverlappedPawn = Cast<APawn>(OtherActor);
	if (OverlappedPawn) 
	{
		PathArr[OverlappedTileIndex].bPawn = true;
	}
	else 
	{
		PathArr[OverlappedTileIndex].bWall = true;
	}
}

void ATileManager::EndTileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) 
{
	FVector TileLocation = OverlappedComponent->GetOwner()->GetActorLocation();
	int32 OverlappedTileIndex = ConvertVectorToIndex(TileLocation);
	APawn* OverlappedPawn = Cast<APawn>(OtherActor);
	if (OverlappedPawn)
	{
		PathArr[OverlappedTileIndex].bPawn = false;
	}
	else
	{
		PathArr[OverlappedTileIndex].bWall = false;
	}
}

/**
* Ÿ������ Wall�� �ִ��� Ȯ��.. 
* TODO
*/
void ATileManager::FindingWallOnTile(ATile* TileActor) 
{
	TArray<AActor*> OverlappedActor;
	TileActor->GetOverlappingActors(OverlappedActor);

	if (OverlappedActor.Num() != 0)
	{
		APawn* OverlappedPawn = Cast<APawn>(OverlappedActor[0]);
		ADestructibleWall* OverlappedDestuctibleWall = Cast<ADestructibleWall>(OverlappedActor[0]);
		if (OverlappedPawn) {}
		else if(OverlappedDestuctibleWall)
		{
			int32 OverlappedTileIndex = ConvertVectorToIndex(TileActor->GetActorLocation());
			PathArr[OverlappedTileIndex].CoverInfo = OverlappedDestuctibleWall->CoverInfo;
			PathArr[OverlappedTileIndex].bWall = true;
		}
		else 
		{
			int32 OverlappedTileIndex = ConvertVectorToIndex(TileActor->GetActorLocation());
			PathArr[OverlappedTileIndex].bWall = true;
		}
		int32 OverlappedTileIndex = ConvertVectorToIndex(OverlappedActor[0]->GetActorLocation());

		UE_LOG(LogTemp, Warning, L"%d Wall On Tile!", OverlappedTileIndex);
	}
}

/**
* Location�� �̿��� Tile�� Index�� �����ϴ�.
* @param WorldLocation - Ÿ���� ���� ��ǥ
* @return Ÿ���� Index�� ��ȯ�մϴ�.
*/
int32 ATileManager::ConvertVectorToIndex(const FVector WorldLocation) 
{
	FVector RelativeLocation = GetActorLocation() - WorldLocation;

	int collum =FMath::Abs(RelativeLocation.X / TileSize);
	int row = FMath::Abs(RelativeLocation.Y / TileSize);
	
	return (row * x) + collum;
}

/**
* Ÿ���� Index�� �̿��� ���� ��ǥ�� �����ϴ�.
* @param Index - Ÿ���� �ε���
* @return Ÿ���� ������ǥ�� ��ȯ�մϴ�.
*/
FVector ATileManager::ConvertIndexToVector(const int32 Index)
{
	int collum = Index % x;
	int row = Index / x;

	return FVector(collum*TileSize, row*TileSize, Root->GetComponentLocation().Z);
}

void ATileManager::ConvertVectorToCoord(FVector WorldLocation, OUT int& CoordX, OUT int& CoordY) {
	int32 TileIndex =  ConvertVectorToIndex(WorldLocation);
	CoordX = TileIndex % x;
	CoordY = TileIndex / y;
}


/**
* �� Ÿ���� �������� �̵� ������ Ÿ�ϵ��� �����ϴ�.
* @param StartingTile - �������̵� Pawn�� �ö��ִ� Ÿ��
* @param MovingAbility - �ൿ�� ( �ִ� �̵� �Ÿ� )
* @param AvailableTiles - �̵� ������ Ÿ�ϵ��� ��� Array
*/
void ATileManager::GetAvailableTiles(ATile* StartingTile, const int32 MovingAbility, int32 MovableStepsPerAct, TArray<ATile*>& AvailableTiles)
{
	TileIndexInRange.Empty();
	int OverlappedTileIndex = ConvertVectorToIndex(StartingTile->GetActorLocation());
	
	TArray<AActor*> ChildActors;
	GetAttachedActors(ChildActors);

	for (int32 i = MovingAbility; i >= -MovingAbility; i--) 
	{
		int32 CurrentStep = MovingAbility - FMath::Abs(i);	//���� ����
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
			else if (PathArr[TargetIndex].bWall == true || PathArr[TargetIndex].bPawn == true)
			{
				continue;
			}
			ATile* TargetTile = Cast<ATile>(ChildActors[ChildActors.Num() - TargetIndex - 1]);
			TileIndexInRange.Add(TargetIndex);
			UE_LOG(LogTemp, Warning , L"���� �� Ÿ�ϵ� %d", TargetIndex)
		}	
	}
	AvailableTiles = FindPath(OverlappedTileIndex, MovingAbility, MovableStepsPerAct, TileIndexInRange);
}

/**
* 
* @param OverlappedTileIndex - Ÿ���� OverlappedTileIndex ��ǥ
* @param RowNumber - Ÿ���� ���� ��ǥ
* @param TargetIndex - Ÿ���� ���� ��ǥ
* @return Ÿ���� Index�� ��ȯ�մϴ�.
*/
bool ATileManager::IsSameLine(const int32 OverlappedTileIndex, const int RowNumber, const int32 TargetIndex)
{
	return ((OverlappedTileIndex / x) + RowNumber) == (TargetIndex / x);
}

/**
* A* �˰��� �̿�Ǵ� �����͵��� �ʱ�ȭ�Ѵ�.
* @param bClearAll - Path�������� ��� ������ �����ϴ� ����
*/
void ATileManager::ClearAllTiles(const bool bClearAll) {
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
			UDecalComponent* Decal = Tile->FindComponentByClass<UDecalComponent>();
			Decal->SetVisibility(false);
			ATile* ActorAsTile = Cast<ATile>(Tile);
			ActorAsTile->bCanMove = false;
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
* Ÿ�ϰ� ����ź �Ÿ��� �����ϴ�.
* @param StartIndex - ���� Ÿ�� �ε���
* @param TargetIndex - ��ǥ Ÿ�� �ε���
* @return ����ź �Ÿ��� ��ȯ�մϴ�.
*/
int32 ATileManager::ComputeManhattanDistance(const int32 StartIndex, const int32 TargetIndex)
{
	int32 CollumDifference = FMath::Abs((StartIndex / x) - (TargetIndex / x));
	int32 RowDifference = FMath::Abs((StartIndex % x) - (TargetIndex % x));

	return (CollumDifference + RowDifference) * TileSize;
}

/**
* �̵� �������� �ִ� �� Ÿ�ϵ鿡 ���ؼ� A* �˰����� ������ ���� �̵������� Ÿ�ϵ��� ����� �����ϴ�.
* @param StartingIndex - �������̵� Pawn�� �ö��ִ� Ÿ��
* @param MovingAbility - �ൿ�� ( �ִ� �̵� �Ÿ� )
* @param TileIndexInRange - �̵� �������� �����ϴ� Ÿ�ϵ�
* @return ���� �̵������� Ÿ�ϵ��� Array
*/
TArray<ATile*> ATileManager::FindPath(const int32 StartingIndex, const int32 MovingAbility, int32 MovableStepsPerAct, TArray<int32> TileIndexInRange)
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
				TargetTile->bCanMove = true;
			}
			else if (PathLength <= MovableStepsPerAct)
			{
				TargetTile->bCanMoveWithOneAct = true;
				AvailableTiles.Add(TargetTile);
				TargetTile->bCanMove = true;
			}
		}
		ClearAllTiles();
	}
	return AvailableTiles;
}

/**
* A* �˰��� ���� ���� ã���ϴ�.
* @param CurrentIndex - ��ã�� �������� Ÿ�� �ε��� 
* @param StartIndex - ���� ������ Ÿ�� �ε���
* @param TargetIndex - ��ǥ ������ Ÿ�� �ε���
* @return ��ǥ ������ Ÿ�Ϸ� �̵����� ���θ� ��ȯ�մϴ�.
*/
bool ATileManager::UpdatePathInfo(const int32 CurrentIndex, const int32 StartIndex , const int32 TargetIndex)
{
	UpdateCardinalPath(CurrentIndex, TargetIndex);
	UpdateDiagonalPath(CurrentIndex, TargetIndex);

	//��������
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
		//F��� ���� �����ɷ� ����
		int NextPathIndex = FindMinCostFIndex();
		OpenList.Remove(NextPathIndex);
		ClosedList.Add(NextPathIndex);

		return UpdatePathInfo(NextPathIndex, StartIndex, TargetIndex);
	}
}

/**
* ��� Cardinal ���⿡ ���ؼ� A*�˰����� �����մϴ�.
* @param CurrentIndex - ��ã�� �������� Ÿ�� �ε��� 
* @param TargetIndex - ��ǥ ������ Ÿ�� �ε���
*/
void ATileManager::UpdateCardinalPath(const int32 CurrentIndex, const int32 TargetIndex)
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
* �ϳ��� Cadinal ������ Tile�� ���ؼ� ��� ����� �մϴ�.
* @param CurrentIndex - ��ã�� �������� Ÿ�� �ε���
* @param CardinalPathIndex - ����꿡 �̿�� Cardinal ������ Ÿ�� �ε���
* @param TargetIndex - ��ǥ ������ Ÿ�� �ε���
*/
void ATileManager::UpdateOneCardinalPath(const int32 CurrentIndex, const int32 CardinalPathIndex, const int32 TargetIndex) {
	int32 RowNumber = 0;
	int32 NewCostG = PathArr[CurrentIndex].CostG + TileSize;
	float PathDecalDirection = 0;

	if (CardinalPathIndex == (CurrentIndex + x)) 
	{
		RowNumber = 1;
		PathDecalDirection = 90;
	}
	else if (CardinalPathIndex == (CurrentIndex - x)) 
	{
		RowNumber = -1;
		PathDecalDirection = 270;
	}
	else 
	{
		RowNumber = 0;

		if (CardinalPathIndex == (CurrentIndex + 1)) 
		{
			PathDecalDirection = 0;
		}
		else if(CardinalPathIndex == (CurrentIndex -1)) 
		{
			PathDecalDirection = 180;
		}
	}
	
	if (CheckWithinBounds(CardinalPathIndex) && IsSameLine(CurrentIndex, RowNumber, CardinalPathIndex) && !ClosedList.Contains(CardinalPathIndex) && TileIndexInRange.Contains(CardinalPathIndex)) 
	{
		//Open List �ȿ� �����Ҷ�
		if (OpenList.Contains(CardinalPathIndex)) 
		{
			//���� �ʿ��Ҷ�
			if (PathArr[CardinalPathIndex].CostG > NewCostG) 
			{
				PathArr[CardinalPathIndex].CostG = NewCostG;
				PathArr[CardinalPathIndex].CostF = PathArr[CardinalPathIndex].CostG + PathArr[CardinalPathIndex].CostH;
				PathArr[CardinalPathIndex].ParentIndex = CurrentIndex;
				PathArr[CardinalPathIndex].PathDirection = PathDecalDirection;
			}
		}
		else // OpenList �� �������� �ʾƼ� ���� �߰��ؾ� �ϴ� ���
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
* ��� Diagonal ���⿡ ���ؼ� A*�˰����� �����մϴ�.
* @param CurrentIndex - ��ã�� �������� Ÿ�� �ε���
* @param TargetIndex - ��ǥ ������ Ÿ�� �ε���
*/
void ATileManager::UpdateDiagonalPath(const int32 CurrentIndex, const int32 TargetIndex)
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
* �ϳ��� Diagonal ������ Tile�� ���ؼ� ��� ����� �մϴ�.
* @param CurrentIndex - ��ã�� �������� Ÿ�� �ε���
* @param DiagonalPathIndex - ����꿡 �̿�� Diagonal ������ Ÿ�� �ε���
* @param TargetIndex - ��ǥ ������ Ÿ�� �ε���
*/
void ATileManager::UpdateOneDiagonalPath(const int32 CurrentIndex, const int32 DiagonalPathIndex, const int32 TargetIndex) {
	
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
		PathDecalDirection = 45;
	}
	else if (DiagonalPathIndex == NorthWestIndex) 
	{
		FromCurrentToDiagonal = 1;
		RowDifference = -x;
		CollumDifference = 1;
		PathDecalDirection = 135;
	}
	else if (DiagonalPathIndex == SouthEastIndex) 
	{
		FromCurrentToDiagonal = -1;
		RowDifference = x;
		CollumDifference = -1;
		PathDecalDirection = 315;
	}
	else if (DiagonalPathIndex == SouthWestIndex) 
	{
		FromCurrentToDiagonal = -1;
		RowDifference = x;
		CollumDifference = 1;
		PathDecalDirection = 225;
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
		TileIndexInRange.Contains(DiagonalPathIndex + CollumDifference) && !ClosedList.Contains(DiagonalPathIndex) && !PathArr[DiagonalPathIndex].bWall && !PathArr[DiagonalPathIndex].bPawn)
	{

		//�̹� OpenList �� �����ϴ� Tile�ϰ�� ���� ����
		if (OpenList.Contains(DiagonalPathIndex))
		{
			// CostG�� �� ���  ������ �ʿ��� ���
			if (PathArr[DiagonalPathIndex].CostG > NewCostG)
			{
				PathArr[DiagonalPathIndex].CostG = PathArr[CurrentIndex].CostG + (int)(TileSize*1.5);
				PathArr[DiagonalPathIndex].CostF = PathArr[DiagonalPathIndex].CostG + PathArr[DiagonalPathIndex].CostH;
				PathArr[DiagonalPathIndex].ParentIndex = CurrentIndex;
				PathArr[DiagonalPathIndex].PathDirection = PathDecalDirection;
			}
		}
		else // OpenList �� �������� �ʾƼ� ���� �߰��ؾ� �ϴ� ���
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
* Ÿ���� �ε����� �ִ� ������ �Ѿ���� Ȯ���մϴ�.
* @param TileIndex - �˻��ϰ� �� Ÿ���� index
* @return �ش� Ÿ���� ��ȿ ������ ������� ���θ� ��ȯ�մϴ�.
*/
bool ATileManager::CheckWithinBounds(const int32 TileIndex)
{
	return  (0 <= TileIndex) && (TileIndex < (x * y));
}

/**
* F ����� ���� ���� ���� Index�� �����ϴ�
* @return F ����� ���� ���� ���� Index
*/
int32 ATileManager::FindMinCostFIndex() {
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
* Ÿ������ ��Ÿ���� ȭ��ǥ Decal ������ �����ϰ�, ���ü��� �����մϴ�.
* @param PathInfo - Key : Ÿ��(key)���� ���� Ÿ�Ϸ�  �̵�����(value)�� �����ִ� Map
* @param bVisibility - Decal�� ������ ���� �����մϴ�.
*/
void ATileManager::SetDecalVisibilityOnTile(TMap<int32, float> PathInfo , const bool bVisibility)
{
	if (bVisibility == false) 
	{
		PathIndicator->ClearAllTile();
		return;
	}

	TArray<FTransform> PathIndicatorTransformArr;
	TArray<AActor*> ChildActors;
	GetAttachedActors(ChildActors);
	
	//PathIndicator
	for (auto OnePathInfo : PathInfo) 
	{
		
		int32 TargetTileIndex = OnePathInfo.Key;
		ATile* Tile = Cast<ATile>(ChildActors[ChildActors.Num() - TargetTileIndex - 1]);

		FTransform TempTransform = FTransform(FRotator(0, OnePathInfo.Value, 0), Tile->GetActorLocation() + FVector(0,0,40));

		/*Tile->SetDecalVisibility(bVisibility);
		if (bVisibility) {
			Tile->SetDecalRotationYaw(OnePathInfo.Value);
		}*/
		PathIndicatorTransformArr.Add(TempTransform);
	}
	PathIndicator->IndicateDirection(PathIndicatorTransformArr);
}


void ATileManager::MouseOnTile(UPrimitiveComponent* OverlappedComponent) {
	ATile* OverlappedTile = Cast<ATile>(OverlappedComponent->GetOwner());
	if (!OverlappedTile) { return; };
	FVector ActorLocation = OverlappedTile->GetActorLocation();
	
	if (OverlappedTile->bCanMove) 
	{
		
	}

	int32 TileIndex = ConvertVectorToIndex(ActorLocation);

	SetDecalVisibilityOnTile(PathArr[TileIndex].OnTheWayMap,  true);
	MakingCoverNotice(TileIndex);

};

void ATileManager::EndMouseOnTile(UPrimitiveComponent* OverlappedComponent) {
	FVector ActorLocation = OverlappedComponent->GetOwner()->GetActorLocation();
	int32 TileIndex = ConvertVectorToIndex(ActorLocation);

	CoveringChecker->ClearAllCoverNotice();

	SetDecalVisibilityOnTile(PathArr[TileIndex].OnTheWayMap, false);
};


ATile* ATileManager::GetOverlappedTile(APawn* Pawn)
{

	TArray<AActor*> OverlappedTileArray;
	Pawn->GetOverlappingActors(OverlappedTileArray);
	if (OverlappedTileArray.Num() == 0) { return nullptr; }

	ATile* Tile = Cast<ATile>(OverlappedTileArray[0]);
	if (!Tile) { return nullptr; }

	return Tile;
}

bool ATileManager::CheckWallAround(const FVector TileLocation, TArray<FVector>& CoverDirectionArr) 
{
	int32 TileIndex = ConvertVectorToIndex(TileLocation);

	int32 EastIndex = TileIndex + 1;
	int32 WestIndex = TileIndex - 1;
	int32 SouthIndex = TileIndex - GetGridXLength();
	int32 NorthIndex = TileIndex + GetGridXLength();

	bool bWallAround = false;
	CheckWallAroundOneDirection(TileIndex, EastIndex, CoverDirectionArr);
	CheckWallAroundOneDirection(TileIndex, SouthIndex, CoverDirectionArr);
	CheckWallAroundOneDirection(TileIndex, NorthIndex, CoverDirectionArr);
	CheckWallAroundOneDirection(TileIndex, WestIndex, CoverDirectionArr);
	if (CoverDirectionArr.Num() != 0 ) 
	{
		bWallAround = true;
	}

	return bWallAround;
}

void ATileManager::CheckWallAroundOneDirection(const int32 TileIndex, const int CardinalIndex, TArray<FVector>& CoverDirectionArr)
{
	int32 RowNumber = 0;
	if (CardinalIndex == (TileIndex + GetGridXLength()))
	{
		RowNumber = 1;
	}
	else if (CardinalIndex == (TileIndex - GetGridXLength()))
	{
		RowNumber = -1;
	}
	else
	{
		RowNumber = 0;
	}

	if (CheckWithinBounds(CardinalIndex) && IsSameLine(TileIndex, RowNumber, CardinalIndex) &&
		PathArr[CardinalIndex].bWall)
	{
		FVector TileLocation = ConvertIndexToVector(TileIndex);
		FVector TargetTileLocation = ConvertIndexToVector(CardinalIndex);
		FVector DirectionToTarget = (TargetTileLocation - TileLocation).GetSafeNormal2D();


		CoverDirectionArr.Add(DirectionToTarget);
	}
}

void ATileManager::MakingCoverNotice(int32 OriginTileIndex)
{
	TArray<FVector> AvailableTilesLocation;

	for (int row = 1; row >= -1; row--) 
	{
		for (int column = 1; column >= -1; column--) 
		{
			int32 TargetTileIndex = OriginTileIndex + column + row*x;
			if (CheckAvailability(TargetTileIndex))
			{
				FVector TargetTileLocation = ConvertIndexToVector(TargetTileIndex) + FVector(-50,-50,50);
				AvailableTilesLocation.Add(TargetTileLocation);
			}
		}
	}
	CoveringChecker->MakingCoverNotice(AvailableTilesLocation, TileSize);

}


bool ATileManager::CheckAvailability(int32 TileIndex)  
{
	bool bAvailability;
	TArray<AActor*> ChildActors;
	GetAttachedActors(ChildActors);

	if (!CheckWithinBounds(TileIndex)) 
	{
		return false;
	}
	ATile* TargetTile = Cast<ATile>(ChildActors[ChildActors.Num() - TileIndex - 1]);
	bAvailability = TargetTile->GetTileVisibility();

	return bAvailability;
}
