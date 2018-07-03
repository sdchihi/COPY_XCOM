// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyUnit.h"
#include "Kismet/KismetSystemLibrary.h"
#include "CustomThirdPerson.h"
#include "XCOMGameMode.h"

AEnemyUnit::AEnemyUnit() 
{
	WalkingState = EWalkingState::Walk;
}

void AEnemyUnit::FinishMoving() 
{

	//여기 밑으론 일단 매번하면 안되요요오
	TArray<AActor*> OutActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> Fillter;
	if (!bAggro) 
	{
		bool bFliteredUnit = UKismetSystemLibrary::SphereOverlapActors
		(
			GetWorld(),
			GetActorLocation(),
			AttackRadius,
			Fillter,
			ACustomThirdPerson::StaticClass(),
			TArray<AActor*>(),
			OutActors
		);

		if (bFliteredUnit)
		{
			UE_LOG(LogTemp,Warning,L"발견!")

			AXCOMGameMode* GameMode = Cast<AXCOMGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
			GameMode->ChangeEnemyAggro(Group);
		}
	}
	//여기까지 아직 Point 사용전. 즉 Cut scene event 처리하기 위한 위치 

	Super::FinishMoving();

}
	

float AEnemyUnit::TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser) 
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	AXCOMGameMode* GameMode = Cast<AXCOMGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode->GetEnemysBattleCognition() == false) 
	{
		GameMode->SetEnemysBattleCognition();
	}

	if (bAggro == false) 
	{
		GameMode->ChangeEnemyAggro(Group);
		if (PlayAggroEventDelegate.IsBound())
		{
			PlayAggroEventDelegate.Execute(this);
		}
	}

	return ActualDamage;
}

void AEnemyUnit::HideUnit()
{
	SetActorHiddenInGame(true);
	GunReference->SetActorHiddenInGame(true);
	HealthBar->SetVisibilityLocker(true);
	HealthBar->SetWidgetVisibility(true);
};


void AEnemyUnit::UnHideUnit()
{
	SetActorHiddenInGame(false);
	GunReference->SetActorHiddenInGame(false);
	HealthBar->SetVisibilityLocker(false);
	HealthBar->SetWidgetVisibility(true);

	AXCOMGameMode* GameMode = Cast<AXCOMGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode->GetEnemysBattleCognition() == true)
	{
		GameMode->ChangeEnemyAggro(Group);
		if (PlayAggroEventDelegate.IsBound()) 
		{
			PlayAggroEventDelegate.Execute(this);
		}
		//Play Aggro Animation - > Anim Notify에 의한 이벤트 발생 고려
	}

}
