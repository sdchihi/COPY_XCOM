// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitHUD.h"
#include "CustomThirdPerson.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/HorizontalBox.h"

void UUnitHUD::NativeConstruct()
{

	Super::NativeConstruct();
}

void UUnitHUD::RegisterActor(ACustomThirdPerson* ActorToSet)
{
	HPBarBox = Cast<UHorizontalBox>(GetWidgetFromName(FName("HorizontalHPBox")));
	
	CharacterRef = ActorToSet;
	InitializeHPBar();
}


void UUnitHUD::InitializeHPBar()
{
	if (HealthPointBlueprint) 
	{
		for (int i = 0; i < CharacterRef->MaxHP; i++) 
		{
			UUserWidget* HealthPoint = CreateWidget<UUserWidget>(GetWorld(), HealthPointBlueprint.Get());

			UHorizontalBoxSlot* HPSlot = HPBarBox->AddChildToHorizontalBox(HealthPoint);
			
			FSlateChildSize Size = FSlateChildSize(ESlateSizeRule::Fill);
			HPSlot->SetSize(Size);
			UE_LOG(LogTemp, Warning , L"Wiget Ãß°¡")
		}
		UE_LOG(LogTemp, Warning, L"Child : %d", HPBarBox->GetChildrenCount());

	}
}
