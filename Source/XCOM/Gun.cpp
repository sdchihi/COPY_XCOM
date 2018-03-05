// Fill out your copyright notice in the Description page of Project Settings.

#include "Gun.h"
#include "Projectile.h"
#include "Classes/Components/ArrowComponent.h"

AGun::AGun() {
	//StaticMeshComponentRef = Cast<UStaticMeshComponent>(RootComponent);

}


void AGun::BeginPlay() {
	Super::BeginPlay();


	

}

void AGun::Fire() 
{
	UE_LOG(LogTemp, Warning, L"Fire!!!");
}