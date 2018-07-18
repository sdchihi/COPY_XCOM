// Fill out your copyright notice in the Description page of Project Settings.

#include "FloatingWidget.h"


void UFloatingWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UFloatingWidget::ShowCombatInfo(AActor* DamagedActor, float Damage)
{
	int8 DamageAsInteger = (int8)Damage;
	SetVisibility(ESlateVisibility::Visible);
	// Todo Something
	if (DamageAsInteger == 0)
	{
		// 회피 , 빗나감
	}
	else 
	{
		// Text  = DamageAsInteger, 데미지
	}

}

void UFloatingWidget::HideWidget() 
{
	// Todo - Hide Widget After Play Animation 
}
