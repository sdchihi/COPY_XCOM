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

	//���� ������ �ϴ� �Ź��ϸ� �ȵǿ���
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
			UE_LOG(LogTemp,Warning,L"�߰�!")

			AXCOMGameMode* GameMode = Cast<AXCOMGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
			GameMode->ChangeEnemyAggro(Group);
		}
	}
	
}


