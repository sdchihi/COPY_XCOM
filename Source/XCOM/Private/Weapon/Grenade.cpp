// Fill out your copyright notice in the Description page of Project Settings.

#include "Grenade.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"



AGrenade::AGrenade()
{
	SphereCollision = CreateDefaultSubobject<USphereComponent>(FName(L"Sphere Collision"));
	ProjectileComponent = CreateDefaultSubobject<UProjectileMovementComponent>(FName("Projectile Movement Component"));

	ProjectileComponent->InitialSpeed = 0;
	ProjectileComponent->MaxSpeed = 0;
	ProjectileComponent->Velocity = FVector(0, 0, 0);
	ProjectileComponent->Bounciness = 0.2;


}


void AGrenade::BeginPlay() 
{

}

