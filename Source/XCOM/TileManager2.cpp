// Fill out your copyright notice in the Description page of Project Settings.

#include "TileManager2.h"
#include "Classes/Components/InstancedStaticMeshComponent.h"
#include "Classes/Components/ChildActorComponent.h"
#include "Classes/GameFramework/Actor.h"
#include "iostream"
// Sets default values
ATileManager2::ATileManager2()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(FName("Root"));
	SetRootComponent(Root);
	
/*
	Tiles = CreateDefaultSubobject<UInstancedStaticMeshComponent>(FName("Tile Instanced Mesh"));
	Tiles->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
*/
	//UChildActorComponent* ChildActor = CreateDefaultSubobject<UChildActorComponent>(FName("Child"));



	//UChildActorComponent child;
	//child.SetChildActorClass()
}

// Called when the game starts or when spawned
void ATileManager2::BeginPlay()
{
	Super::BeginPlay();
	
	for (int i = 0; i < (x*y); i++) {
		int collum = i % x;
		int row = i / x;
		
		FTransform instanceTransform = FTransform(FVector(collum*TileSize*110, row*TileSize*110, Root->GetComponentLocation().Z));
		instanceTransform.SetScale3D(FVector(TileSize, TileSize, 1));

		AActor* actor = GetWorld()->SpawnActor<AActor>(
			WallBlueprint,
			FVector(collum*TileSize * 110, row*TileSize * 110, Root->GetComponentLocation().Z),
			FRotator(0,0,0)
			);

		if (!actor) {
			UE_LOG(LogTemp, Warning, L"Wall BP 지정 필요");
			return;
		}
		actor->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
		
		UStaticMeshComponent* ActorMeshComponent = actor->FindComponentByClass<UStaticMeshComponent>();

		ActorMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &ATileManager2::OnOverlapBegin);
		
		
		//actor->OnActorHit.AddDynamic(this, &ATileManager2::OnOverlapBegin);

		

		//Path 초기화
		Path DefaultPathValue;
		DefaultPathValue.bWall = false;
		DefaultPathValue.Cost = 0;
		PathArr.Add(DefaultPathValue);
	}
}

// Called every frame
void ATileManager2::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}



void ATileManager2::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult){
	UE_LOG(LogTemp, Warning, L"ABCCEE!");
	UE_LOG(LogTemp, Warning, L"ABCCEE!");
}
