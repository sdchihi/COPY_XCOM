// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile.h"
#include "Classes/GameFramework/ProjectileMovementComponent.h"

AProjectile::AProjectile() 
{

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(FName("ProjectileMovement"));

}


void AProjectile::BeginPlay() {

}


