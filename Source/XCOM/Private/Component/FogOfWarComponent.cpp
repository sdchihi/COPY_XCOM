// Fill out your copyright notice in the Description page of Project Settings.

#include "FogOfWarComponent.h"
#include "FogOfWarManager.h"
#include "XCOMGameMode.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "CustomThirdPerson.h"

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
	ACustomThirdPerson* Unit = Cast<ACustomThirdPerson>(GetOwner());
	if (Unit) 
	{
		if (bCanCognize) 
		{
			Unit->UnHideUnit();
		}
		else 
		{
			Unit->HideUnit();
		}
	}
	isActorInTerraIncog = bCanCognize;
}

