// Fill out your copyright notice in the Description page of Project Settings.

#include "Wall.h"
#include "Classes/Components/StaticMeshComponent.h"

// Sets default values
AWall::AWall()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


}

// Called when the game starts or when spawned
void AWall::BeginPlay()
{
	Super::BeginPlay();

	UStaticMeshComponent* RootMeshComponent = Cast<UStaticMeshComponent>(GetRootComponent());
	if (!ensure(RootMeshComponent)) { 
		UE_LOG(LogTemp, Warning, L"!!!");
		return; }
	RootMeshComponent->OnComponentHit.AddDynamic(this, &AWall::OnHit);


	UE_LOG(LogTemp, Warning, L"!!!2");

}

// Called every frame
void AWall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}





void AWall::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit) {
	UE_LOG(LogTemp, Warning, L"ABCCEE!");
}
