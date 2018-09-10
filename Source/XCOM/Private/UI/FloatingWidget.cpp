// Fill out your copyright notice in the Description page of Project Settings.

#include "FloatingWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Classes/Engine/Texture2D.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "Classes/GameFramework/PlayerController.h"

void UFloatingWidget::NativeConstruct()
{
	Super::NativeConstruct();
	FigureText = Cast<UTextBlock>(GetWidgetFromName(FName("Damage")));
	ExplanationText = Cast<UTextBlock>(GetWidgetFromName(FName("Explanation")));
	BackgroundImage = Cast<UImage>(GetWidgetFromName(FName("ColoredBackground")));
	IconImage = Cast<UImage>(GetWidgetFromName(FName("Icon")));	
}

/**
* 전투 결과를 팝업시킵니다.
* @param Damage - 데미지 값
* @param State - 전투 정보
*/
void UFloatingWidget::ShowCombatInfo(float Damage, FloatingWidgetState State)
{
	int8 DamageAsInteger = (int8)Damage;
	SetVisibility(ESlateVisibility::Visible);

	ChangePopUpFactor(State, DamageAsInteger);
	float EndTime = PlayAnimationByName(FName("PopUp"));
	FTimerHandle UnUsedHandle;
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &UFloatingWidget::PopDown);
	GetWorld()->GetTimerManager().SetTimer(UnUsedHandle, TimerDelegate, EndTime + 2.5, false);
}

/**
* 팝업된 전투 결과를 사라지게 합니다.
*/
void UFloatingWidget::PopDown()
{
	float EndTime = PlayAnimationByName(FName("Disapear"));
	FTimerHandle UnUsedHandle;
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &UFloatingWidget::HideWidget);
	GetWorld()->GetTimerManager().SetTimer(UnUsedHandle, TimerDelegate, EndTime, false);
}

void UFloatingWidget::HideWidget()
{
	SetVisibility(ESlateVisibility::Hidden);
}

/**
* Widget의 이미지, 텍스트를 변경합니다.
* @param State - 전투 정보
* @param Damage - 데미지 값
*/
void UFloatingWidget::ChangePopUpFactor(FloatingWidgetState State, int8 Damage)
{
	FSlateBrush BackgroundBrush;
	FSlateBrush IconBrush;
	FSlateColor BackgroundColor;
	FString ImagePath;
	
	switch (State) 
	{
	case FloatingWidgetState::Damaged:
		BackgroundColor = FSlateColor(FLinearColor(0.465, 0, 0));
		ExplanationText->SetText(FText::FromString("Damage"));
		FigureText->SetText(FText::AsNumber(Damage));
		ImagePath = "/Game/Texture/PopUp/Hit.Hit";
		break;
	case FloatingWidgetState::Dodge:
		BackgroundColor = FSlateColor(FLinearColor(0.465, 0, 0));
		ExplanationText->SetText(FText::FromString("Missed!"));
		FigureText->SetText(FText::FromString(""));
		ImagePath = "/Game/Texture/PopUp/Dodge.Dodge";
		break;
	case FloatingWidgetState::Visilance:
		BackgroundColor = FSlateColor(FLinearColor(0, 0.15, 0.78));
		ExplanationText->SetText(FText::FromString("Visilance"));
		FigureText->SetText(FText::FromString(""));
		ImagePath = "/Game/Texture/PopUp/Visilance.Visilance";
		break;
	case FloatingWidgetState::Critical:
		BackgroundColor = FSlateColor(FLinearColor(0.78, 0.75, 0));
		ExplanationText->SetText(FText::FromString("Critical"));
		FigureText->SetText(FText::AsNumber(Damage));
		ImagePath = "/Game/Texture/PopUp/Critical.Critical";
		break;
	default:
		return;
	}
	BackgroundBrush.TintColor = BackgroundColor;
	BackgroundBrush.ImageSize = FVector2D(90, 32);
	BackgroundImage->SetBrush(BackgroundBrush);
	
	UTexture2D* IconTexture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), NULL, *(ImagePath)));
	IconBrush.ImageSize = FVector2D(45, 45);
	IconBrush.SetResourceObject(IconTexture);
	IconImage->SetBrush(IconBrush);
}
