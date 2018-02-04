// Fill out your copyright notice in the Description page of Project Settings.

#include "Obstacle.h"
#include "Classes/Components/StaticMeshComponent.h"

void  AObstacle::BeginPlay() {
	Super::BeginPlay();
	PrimaryActorTick.bCanEverTick = true;



	UStaticMeshComponent* RootMeshComponent = Cast<UStaticMeshComponent>(GetRootComponent());
	if (!ensure(RootMeshComponent)) {
		UE_LOG(LogTemp, Warning, L"!!!");
		return;
	}
	//RootMeshComponent->OnComponentHit.AddDynamic(this, &AObstacle::OnHit);


	UE_LOG(LogTemp, Warning, L"!!!2");

};

void  AObstacle::Tick(float DeltaTime) {

	Super::Tick(DeltaTime);

	//UStaticMeshComponent* RootMeshComponent = Cast<UStaticMeshComponent>(GetRootComponent());
	//TArray<AActor*> temp;

	//RootMeshComponent->GetOverlappingActors(temp);
	//UE_LOG(LogTemp, Warning, L"%d", temp.GetSlack());
	//temp.GetSlack();
};




void AObstacle::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit) {
	UE_LOG(LogTemp, Warning, L"ABCCEE!");
}