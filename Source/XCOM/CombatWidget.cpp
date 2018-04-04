// Fill out your copyright notice in the Description page of Project Settings.

#include "CombatWidget.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "XCOMPlayerController.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Misc/Paths.h"
#include "Classes/Engine/Texture2D.h"
#include "CustomButton.h"

void UCombatWidget::InitializeInBP() 
{
	RightVBox = Cast<UVerticalBox>(GetWidgetFromName(FName("RightContentsVBox")));
	LeftVBox = Cast<UVerticalBox>(GetWidgetFromName(FName("LeftContentsVBox")));
	CenterActionHBox = Cast<UHorizontalBox>(GetWidgetFromName(FName("ActionHBox")));
	EnemyIconHBox = Cast<UHorizontalBox>(GetWidgetFromName(FName("EnemyListHBox")));

	AXCOMPlayerController* PlayerController = Cast<AXCOMPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	PlayerController->DeleverInfoDelegate.BindDynamic(this, &UCombatWidget::Renew);

};

void UCombatWidget::ClearContents(const bool bClearAll)
{
	RightVBox->ClearChildren();
	LeftVBox->ClearChildren();
	CenterActionHBox->ClearChildren();

	if (bClearAll) 
	{
		EnemyIconHBox->ClearChildren();
		SelectedCharacterAimingInfo.Empty();
	}
}

void UCombatWidget::Renew(const TArray<FAimingInfo>& AimingInfoArray, const FPossibleActionWrapper& PossibleActionMapWrapper)
{
	ClearContents(true);

	SelectedCharacterAimingInfo = AimingInfoArray;
	PossibleActionMap = PossibleActionMapWrapper.PossibleAction;

	FString Explanation;
	float Percentage; 
	
	//Fill Side Contents 
	ConvertToSuitableFormat(SelectedCharacterAimingInfo[0], Explanation, Percentage); 
	// 상단 Enemy Icon
	FillEnemyList();
	FillActionButtonList();
}

void UCombatWidget::ConvertToSuitableFormat(const FAimingInfo& AimingInfo, OUT FString& Explanation, OUT float& Percentage) 
{
	for (auto SingleFactor : AimingInfo.Factor) 
	{
		switch (SingleFactor.Key)
		{
		case EAimingFactor::AimingAbility:
			Explanation = L"조준";
			break;
		case EAimingFactor::FullCover:
			Explanation = L"완전 엄폐";
			break;
		case EAimingFactor::HalfCover:
			Explanation = L"부분 엄폐";
			break;
		case EAimingFactor::GoodAngle:
			Explanation = L"좋은 각도";
			break;
		case EAimingFactor::Disatnce:
			Explanation = L"거리";
			break;
		default:
			break;
		}
		Percentage = SingleFactor.Value * 100;
		FillContnents(Explanation, Percentage);
	}
}

void UCombatWidget::FillContnents(const FString& Explanation, const float Percentage)
{
	if (SideContentBoxBlueprint)
	{
		//굉장히 유연하지 못한 상태 이후에 고쳐야함.
		UUserWidget* LeftContentsBox = CreateWidget<UUserWidget>(GetWorld(), SideContentBoxBlueprint.Get());
		UTextBlock* CommentText = Cast<UTextBlock>(LeftContentsBox->GetWidgetFromName(FName("Comment")));
		UTextBlock* PercentageText = Cast<UTextBlock>(LeftContentsBox->GetWidgetFromName(FName("PercentageValue")));
		CommentText->SetText(FText::FromString(Explanation));
		PercentageText->SetText(FText::AsNumber(Percentage));

		LeftVBox->AddChild(LeftContentsBox);
	}
}


void UCombatWidget::FillEnemyList() 
{
	for (int i = 0; i < SelectedCharacterAimingInfo.Num(); i++) 
	{
		UCustomButton* ButtonForMarkingEnemy = NewObject<UCustomButton>(UCustomButton::StaticClass());;
		FString ImagePath = "/Game/Texture/EnemyIcon.EnemyIcon";
		UTexture2D* Texture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), NULL, *(ImagePath)));
		FButtonStyle ButtonStyle;

		FSlateBrush TempBrush;
		TempBrush.SetResourceObject(Texture);
		TempBrush.DrawAs = ESlateBrushDrawType::Image;
		ButtonStyle.SetNormal(TempBrush);
		ButtonForMarkingEnemy->SetStyle(ButtonStyle);
		ButtonForMarkingEnemy->SetButtonIndex(i);
		ButtonForMarkingEnemy->ConveyIndexDelegate.BindDynamic(this, &UCombatWidget::EnemyButtonClicked);
		
		EnemyIconHBox->AddChild(ButtonForMarkingEnemy);
	}
}

void UCombatWidget::EnemyButtonClicked(int32 ButtonIndex) 
{
	ClearContents();

	FString Explanation;
	float Percentage;
	ConvertToSuitableFormat(SelectedCharacterAimingInfo[ButtonIndex], Explanation, Percentage);

	ChangeViewTargetDelegate.Execute(SelectedCharacterAimingInfo[ButtonIndex].TargetLocation);
}


void UCombatWidget::FillActionButtonList() 
{
	for (auto SinglePossibleAction : PossibleActionMap) 
	{
		FString ButtonImagePath;
		switch (SinglePossibleAction.Key) 
		{
		case EAction::Attack:
			ButtonImagePath = "";
			break;
		case EAction::Ambush:
			ButtonImagePath = "";
			break;
		case EAction::Grenade:
			ButtonImagePath = "";
			break;
		case EAction::Vigilance:
			ButtonImagePath = "";
			break;
		case EAction::None:
			ButtonImagePath = "";
			break;
		}

		if (SinglePossibleAction.Value == true)
		{
			UTexture2D* Texture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), NULL, *(ButtonImagePath)));
			
			UCustomButton* ButtonForMarkingEnemy = NewObject<UCustomButton>(UCustomButton::StaticClass());;
			FSlateBrush TempBrush;
			TempBrush.SetResourceObject(Texture);
			TempBrush.DrawAs = ESlateBrushDrawType::Image;

			FButtonStyle ButtonStyle;
			ButtonStyle.SetNormal(TempBrush);
			ButtonForMarkingEnemy->SetStyle(ButtonStyle);

			CenterActionHBox->AddChild(ButtonForMarkingEnemy);
		}
	}
}
