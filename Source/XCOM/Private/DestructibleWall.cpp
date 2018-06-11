// Fill out your copyright notice in the Description page of Project Settings.

#include "DestructibleWall.h"
#include "Classes/Components/DestructibleComponent.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "Projectile.h"
#include "Classes/Components/BoxComponent.h"

ADestructibleWall::ADestructibleWall() 
{

	DestructibleCompReference = FindComponentByClass<UDestructibleComponent>();
	if (DestructibleCompReference) {
		//DestructibleCompReference->SetSimulatePhysics(true);
		DestructibleCompReference->OnComponentHit.AddDynamic(this, &ADestructibleWall::OnHit);
		//DestructibleCompReference->SetNotifyRigidBodyCollision(true); // 확인 필요   Hit Event 발생 여부
	}
	

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(FName("Box Collision"));
	BoxCollision->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	
}

void ADestructibleWall::BeginPlay() {
	Super::BeginPlay();

	FVector Origin;
	FVector BoxExtent;
	GetActorBounds(false, Origin, BoxExtent);
	BoxCollision->SetRelativeLocation(FVector(0, 0,-BoxExtent.Z / 2));

	switch(CoverInfo) 
	{
	case ECoverInfo::FullCover:
		Durability = MaxDurability;
		break;
	case ECoverInfo::HalfCover:
		Durability = MaxDurability / 2;
		break;
	}


}



void ADestructibleWall::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit) 
{
	TSubclassOf<UDamageType> DamageType;
	FHitResult HitInfo;
	AProjectile* ProjectileActor = Cast<AProjectile>(OtherActor);


	if (ProjectileActor) {
		//UGameplayStatics::ApplyPointDamage(OtherActor, ProjectileActor->GetDamage() , GetActorLocation(), HitInfo, nullptr, this, DamageType);
		UGameplayStatics::ApplyDamage(this, ProjectileActor->GetDamage(), nullptr, OtherActor, DamageType);
		UE_LOG(LogTemp, Warning, L"Projectile Hit");
	}
	else {
		UE_LOG(LogTemp, Warning, L"Not Projectile Hit");
	}
}
//void ADestructibleWall::GetCollisionObjType() 
//{
//}