// Fill out your copyright notice in the Description page of Project Settings.

#include "CombatWidget.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "CustomThirdPerson.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "XCOMPlayerController.h"

bool UCombatWidget::Initialize()
{
	bool ReturnValue = Super::Initialize();

	RightVBox =Cast<UVerticalBox>(GetWidgetFromName(FName("RightContentsVBox")));
	LeftVBox = Cast<UVerticalBox>(GetWidgetFromName(FName("LeftContentsVBox")));
	CenterActionHBox = Cast<UHorizontalBox>(GetWidgetFromName(FName("ActionHBox")));

	AXCOMPlayerController* PlayerController =  Cast<AXCOMPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	PlayerController->DeleverInfoDelegate.BindDynamic(this, &UCombatWidget::Renew);


	return ReturnValue;
}


void UCombatWidget::ClearContents() 
{
	RightVBox->ClearChildren();
	LeftVBox->ClearChildren();
	CenterActionHBox->ClearChildren();
	SelectedCharacterAimingInfo.Empty();
}


void UCombatWidget::Renew(const TArray<FAimingInfo>& AimingInfoArray)
{
	ClearContents();

	SelectedCharacterAimingInfo = AimingInfoArray;
	
	FString Explanation;
	float Percentage;
	ConvertToSuitableFormat(SelectedCharacterAimingInfo[0], Explanation, Percentage);
	//FillContnents(Explanation, Percentage);
	
}

void UCombatWidget::ConvertToSuitableFormat(const FAimingInfo& AimingInfo, OUT FString& Explanation, OUT float& Percentage) 
{
	for (auto SingleFactor : AimingInfo.Factor) 
	{
		switch (SingleFactor.Key)
		{
		case EAimingFactor::AimingAbility:
			Explanation = "����";
			break;
		case EAimingFactor::FullCover:
			Explanation = "���� ����";
			break;
		case EAimingFactor::HalfCover:
			Explanation = "�κ� ����";
			break;
		case EAimingFactor::GoodAngle:
			Explanation = "���� ����";
			break;
		case EAimingFactor::Disatnce:
			Explanation = "�Ÿ�";
			break;
		default:
			break;
			Percentage = SingleFactor.Value * 100;
		}
		FillContnents(Explanation, Percentage);
	}
}

void UCombatWidget::FillContnents(const FString& Explanation, const float Percentage)
{
	if (SideContentBoxBlueprint)
	{
		//������ �������� ���� ���� ���Ŀ� ���ľ���.
		UUserWidget* LeftContentsBox = CreateWidget<UUserWidget>(GetWorld(), SideContentBoxBlueprint.Get());
		UTextBlock* CommentText = Cast<UTextBlock>(LeftContentsBox->GetWidgetFromName(FName("Comment")));
		UTextBlock* PercentageText = Cast<UTextBlock>(LeftContentsBox->GetWidgetFromName(FName("PercentageValue")));
	
		CommentText->SetText(FText::FromString(Explanation));
		PercentageText->SetText(FText::AsNumber(Percentage));

		LeftVBox->AddChild(LeftContentsBox);
	}
}
