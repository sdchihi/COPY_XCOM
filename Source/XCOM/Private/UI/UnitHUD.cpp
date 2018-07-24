// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitHUD.h"
#include "CustomThirdPerson.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "CustomUserWidget.h"
#include "Components/Image.h"
#include "Classes/Engine/Texture2D.h"

void UUnitHUD::NativeConstruct()
{

	Super::NativeConstruct();
}

void UUnitHUD::RegisterActor(ACustomThirdPerson* ActorToSet)
{
	GridPannel = Cast<UUniformGridPanel>(GetWidgetFromName(FName("HPGridPannel")));
	GridPannel->SetSlotPadding(FMargin(2, 2));
	TeamImage = Cast<UImage>(GetWidgetFromName(FName("TeamIcon")));

	CharacterRef = ActorToSet;

	SetTeamIconImage();
	InitializeHPBar();
}


void UUnitHUD::InitializeHPBar()
{
	UClass* HPClass = GetHPClass(CharacterRef->GetTeamFlag());
	if (HPClass) 
	{
		for (int i = 0; i < CharacterRef->MaxHP; i++)
		{
			UCustomUserWidget* HealthPoint = CreateWidget<UCustomUserWidget>(GetWorld(), HPClass);

			UUniformGridSlot* HPSlot = GridPannel->AddChildToUniformGrid(HealthPoint);
			HPSlot->HorizontalAlignment = EHorizontalAlignment::HAlign_Fill;
			HPSlot->VerticalAlignment = EVerticalAlignment::VAlign_Fill;
			HPSlot->SetColumn(i);

			HPWidgetArray.Push(HealthPoint);
		}
	}
	else 
	{
		UE_LOG(LogTemp, Warning, L"HPClass 로딩 실패");
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

void UUnitHUD::SetTeamIconImage()
{
	FSlateBrush IconBrush;
	FString ImagePath;
	if (CharacterRef->GetTeamFlag())
	{
		ImagePath = "/Game/Texture/PlayerUnitIcon.PlayerUnitIcon";
	}
	else 
	{
		ImagePath = "/Game/Texture/EnemyHUDIcon.EnemyHUDIcon";
	}

	UTexture2D* IconTexture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), NULL, *(ImagePath)));
	IconBrush.SetResourceObject(IconTexture);
	TeamImage->SetBrush(IconBrush);
}

UClass* UUnitHUD::GetHPClass(bool bIsPlayerTeam)
{
	FStringClassReference HPClassRef;
	if (bIsPlayerTeam) 
	{
		HPClassRef.SetPath(TEXT("/Game/UI/BP_PlayerHealthPoint.BP_PlayerHealthPoint_C"));
	}
	else 
	{
		HPClassRef.SetPath(TEXT("/Game/UI/BP_EnemyHealthPoint.BP_EnemyHealthPoint_C"));
	}

	return HPClassRef.TryLoadClass<UCustomUserWidget>();
}