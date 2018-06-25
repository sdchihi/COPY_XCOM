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
	Super::FinishMoving();
	EObjectTypeQuery::ObjectTypeQuery1;

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
			GetWolrd()->AddOnScreenDebugMessage(-1, 0.0f, FColor::Cyan, "발겨어어언"); 

			AXCOMGameMode* GameMode = Cast<AXCOMGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
			GameMode->ChangeEnemyAggro(Group);
		}
	}
	
}


