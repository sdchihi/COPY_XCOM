// Fill out your copyright notice in the Description page of Project Settings.

#include "Gun.h"
#include "Projectile.h"
#include "Classes/Components/ArrowComponent.h"

AGun::AGun() {
	FirePos = CreateDefaultSubobject<UArrowComponent>(FName("FirePos"));
	FirePos->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

}


void AGun::BeginPlay() {
	Super::BeginPlay();

}

void AGun::Fire() 
{
	FRotator FireRotation[6] = { FRotator(0, 0, 0), FRotator(2, 0, 0),FRotator(0.5, 1.5, 0) ,FRotator(-1, 1, 0), FRotator(-1, -1, 0), FRotator(0.5, -1.5, 0) };


	if (ProejctileBlueprint) {
		GetWorld()->SpawnActor<AProjectile>(
			ProejctileBlueprint,
			FirePos->GetComponentLocation(),
			FirePos->GetForwardVector().ToOrientationRotator() + FireRotation[FiringRotOrder]
			);
	}

	if (FiringRotOrder == 5) {
		FiringRotOrder = 0;
	}
	else {
		FiringRotOrder++;
	}
}