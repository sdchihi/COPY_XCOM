// Fill out your copyright notice in the Description page of Project Settings.

#include "TileManager2.h"
#include "Classes/Components/InstancedStaticMeshComponent.h"
#include "Classes/Components/ChildActorComponent.h"
#include "Classes/GameFramework/Actor.h"

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

		// Tile ����
		AActor* TileActor = GetWorld()->SpawnActor<AActor>(
			WallBlueprint,
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
		Path DefaultPathValue;
		DefaultPathValue.bWall = false;
		DefaultPathValue.Cost = 0;
		PathArr.Add(DefaultPathValue);

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


void ATileManager2::FindingWallOnTile(AActor* TileActor) 
{
	TArray<AActor*> OverlappedActor;
	TileActor->GetOverlappingActors(OverlappedActor);

	if (OverlappedActor.Num() != 0) {
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



void ATileManager2::GetNearbyTiles(AActor* StartingTile, int32 MovingAbility, TArray<AActor*>& AvailableTiles) {

	int OverlappedTileIndex = ConvertVectorToIndex(StartingTile->GetActorLocation());
	//������� �ߵ�
	
	TArray<AActor*> ChildActors;
	GetAttachedActors(ChildActors);
	ChildActors[OverlappedTileIndex];
	
	UE_LOG(LogTemp, Warning, L"%d", OverlappedTileIndex);


	// start-  0 ,  moving abil = 2


	int CurrentStep = MovingAbility;
	for (int32 i = MovingAbility; i >= -MovingAbility; i--) 
	{
		int32 CurrentStep = MovingAbility - FMath::Abs(i);

		int32 TargetIndex;
		//currentstep ==0 �϶��� ����ó�����ְ� �������� �ؿ� �ݺ������� ����

		// Left To Right - > 
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
			else if(!IsSameLine(OverlappedTileIndex, i , TargetIndex))   //���������� ���̻� Ÿ���� ������ �ʿ������
			{	
				continue;
			}
			AvailableTiles.Add(ChildActors[ChildActors.Num()-TargetIndex-1]);
			

			UE_LOG(LogTemp, Warning, L"Input : %s", *ChildActors[TargetIndex]->GetName());
			
		}	
	}

}



int32 ATileManager2::GetNumOfRightSideExtraTile(int32 TileIndex) 
{
	int32 row = TileIndex / x;  // i �ϰ� �ٸ��� ���� ��Ű��..



	return  (TileIndex) % x;
}


bool ATileManager2::IsSameLine(int32 OverlappedTileIndex, int RowNumber, int32 TargetIndex) 
{
	return ((OverlappedTileIndex / x) + RowNumber) == (TargetIndex / x);
}
