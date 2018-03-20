// Fill out your copyright notice in the Description page of Project Settings.

#include "DestructibleWall.h"
#include "Classes/Components/DestructibleComponent.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "Projectile.h"


ADestructibleWall::ADestructibleWall() 
{

	DestructibleCompReference = FindComponentByClass<UDestructibleComponent>();
	if (DestructibleCompReference) {
		//DestructibleCompReference->SetSimulatePhysics(true);
		DestructibleCompReference->OnComponentHit.AddDynamic(this, &ADestructibleWall::OnHit);
		//DestructibleCompReference->SetNotifyRigidBodyCollision(true); // 확인 필요   Hit Event 발생 여부
	}

}

void ADestructibleWall::BeginPlay() {
	Super::BeginPlay();

}



void ADestructibleWall::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit) {

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
//
//void ADestructibleWall::ChangeObjectType()
//{
//	; 
//}