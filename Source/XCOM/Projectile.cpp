// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Classes/GameFramework/ProjectileMovementComponent.h"

AProjectile::AProjectile() 
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(FName("ProjectileMovement"));
	ProjectileMovementComponent->MaxSpeed = 2000;
	ProjectileMovementComponent->InitialSpeed = 2000;
	ProjectileMovementComponent->ProjectileGravityScale = 0;

	Mesh = Cast<UStaticMeshComponent>(RootComponent);
	if (Mesh) {
		Mesh->SetEnableGravity(false);
		Mesh->SetSimulatePhysics(true);
		Mesh->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
		Mesh->SetNotifyRigidBodyCollision(true); // Ȯ�� �ʿ�   Hit Event �߻� ����
	}
}
	
	


void AProjectile::BeginPlay() {

}


void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit) {
	//UE_LOG(LogTemp, Warning, L"Hit Obj name : %s", *OtherActor->GetName());
	//Todo - ��ƼŬ ���� �� ���� �� Destroy
	//Destroy();
}
