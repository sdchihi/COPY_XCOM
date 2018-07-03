// Fill out your copyright notice in the Description page of Project Settings.

#include "FogOfWarComponent.h"
#include "FogOfWarManager.h"
#include "XCOMGameMode.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "CustomThirdPerson.h"
#include "EnemyUnit.h"

UFogOfWarComponent::UFogOfWarComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFogOfWarComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


void UFogOfWarComponent::SetActorInTerraInCog(bool bCanCognize)
{
	AEnemyUnit* EnemyUnit = Cast<AEnemyUnit>(GetOwner());
	if (EnemyUnit)
	{
		if (bCanCognize) 
		{
			EnemyUnit->UnHideUnit();
		}
		else 
		{
			EnemyUnit->HideUnit();
		}
	}
	isActorInTerraIncog = bCanCognize;
}

