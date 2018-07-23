// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitHUD.h"
#include "CustomThirdPerson.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "CustomUserWidget.h"

void UUnitHUD::NativeConstruct()
{

	Super::NativeConstruct();
}

void UUnitHUD::RegisterActor(ACustomThirdPerson* ActorToSet)
{
	GridPannel = Cast<UUniformGridPanel>(GetWidgetFromName(FName("HPGridPannel")));
	GridPannel->SetSlotPadding(FMargin(2, 2));

	CharacterRef = ActorToSet;
	InitializeHPBar();
}


void UUnitHUD::InitializeHPBar()
{
	if (HealthPointBlueprint)
	{
		for (int i = 0; i < CharacterRef->MaxHP; i++)
		{
			UCustomUserWidget* HealthPoint = CreateWidget<UCustomUserWidget>(GetWorld(), HealthPointBlueprint.Get());

			UUniformGridSlot* HPSlot = GridPannel->AddChildToUniformGrid(HealthPoint);
			HPSlot->HorizontalAlignment = EHorizontalAlignment::HAlign_Fill;
			HPSlot->VerticalAlignment = EVerticalAlignment::VAlign_Fill;
			HPSlot->SetColumn(i);

			HPWidgetArray.Push(HealthPoint);
		}
	}
}

void UUnitHUD::ReduceHP(int8 Damage) 
{
	for (int i = 0; i < Damage; i++) 
	{
		UCustomUserWidget* HPWidget = HPWidgetArray.Pop();
		if (HPWidget) 
		{
			HPWidget->PlayAnimationByName(FName("Damaged"));
		}
	}
}

