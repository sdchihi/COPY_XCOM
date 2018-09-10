// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitHUD.h"
#include "CustomThirdPerson.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "CustomUserWidget.h"
#include "Components/Image.h"
#include "Classes/Engine/Texture2D.h"
#include "Components/CanvasPanel.h"
void UUnitHUD::NativeConstruct()
{

	Super::NativeConstruct();
}

/**
* Actor를 등록합니다.
* @param ActorToSet - 등록할 Actor
*/
void UUnitHUD::RegisterActor(ACustomThirdPerson* ActorToSet)
{
	GridPannel = Cast<UUniformGridPanel>(GetWidgetFromName(FName("HPGridPannel")));
	GridPannel->SetSlotPadding(FMargin(2, 2));
	TeamImage = Cast<UImage>(GetWidgetFromName(FName("TeamIcon")));
	Background = Cast<UImage>(GetWidgetFromName(FName("BackgroundImage")));
	CharacterRef = ActorToSet;

	SetTeamIconImage();
	InitializeHPBar();
}

/**
* HP바를 초기화합니다.
*/
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

/**
* 체력을 감소시킵니다.
* @param Damage - 데미지 값
*/
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

/**
* 팀을 표시하는 이미지를 갱신합니다.
*/
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

/**
* 팀에 맞춰서 사용하게 될 HP Block 클래스를 얻어옵니다.
* @param bIsPlayerTeam - 팀 플래그
*/
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

void UUnitHUD::DestroyHPBar() 
{
	for (auto HealthPointWidget : HPWidgetArray) 
	{
		HealthPointWidget->RemoveFromParent();
	}
	TeamImage->RemoveFromParent();
	Background->SetVisibility(ESlateVisibility::Hidden);
	GridPannel->RemoveFromParent();
	Background->RemoveFromParent();
}