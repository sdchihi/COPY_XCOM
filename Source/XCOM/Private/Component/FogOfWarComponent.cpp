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

/**
* 안개안에 위치하고있으면 Unit을 게임에서 감춥니다.
* @param bCanCognize 안개안에 위치하고있는지 여부
*/
void UFogOfWarComponent::SetActorInTerraInCog(bool bCanCognize)
{
	AEnemyUnit* EnemyUnit = Cast<AEnemyUnit>(GetOwner());
	if (EnemyUnit)
	{
		if (bCanCognize)
		{
			EnemyUnit->HideUnit();
		}
		else 
		{
			EnemyUnit->UnHideUnit();
		}
	}
	isActorInTerraIncog = bCanCognize;
}