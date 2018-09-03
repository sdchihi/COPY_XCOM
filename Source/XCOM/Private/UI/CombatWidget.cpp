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
#include "Components/HorizontalBoxSlot.h"
#include "Components/CanvasPanel.h"
#include "WidgetLayoutLibrary.h"

void UCombatWidget::NativeConstruct()
{
	InitializeInBP();
	// Call Blueprint Event Construct node
	Super::NativeConstruct();
}

void UCombatWidget::InitializeInBP() 
{
	RightVBox = Cast<UVerticalBox>(GetWidgetFromName(FName("RightContentsVBox")));
	LeftVBox = Cast<UVerticalBox>(GetWidgetFromName(FName("LeftContentsVBox")));
	CenterActionHBox = Cast<UHorizontalBox>(GetWidgetFromName(FName("ActionHBox")));
	EnemyIconHBox = Cast<UHorizontalBox>(GetWidgetFromName(FName("EnemyListHBox")));
	MainStartActionButton = Cast<UButton>(GetWidgetFromName(FName("MainActionButton")));
	SumAimingProbBox = Cast<UUserWidget>(GetWidgetFromName(FName("SumAimingProb")));
	SumCriticalProbBox = Cast<UUserWidget>(GetWidgetFromName(FName("SumCriticalProb")));
	UTextBlock* CommentBox = Cast<UTextBlock>(SumCriticalProbBox->GetWidgetFromName(FName("Comment")));
	CommentBox->SetText(FText::FromString(L"치명타"));


	LeftFrame = Cast<UCanvasPanel>(GetWidgetFromName(FName("LeftContainer")));
	RightFrame = Cast<UCanvasPanel>(GetWidgetFromName(FName("RightContainer")));
	CenterFrame = Cast<UCanvasPanel>(GetWidgetFromName(FName("CenterContainer")));

	PlayerController = Cast<AXCOMPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	PlayerController->DeleverInfoDelegate.BindDynamic(this, &UCombatWidget::Renew);
	ConstructWidgetMinimum();
};

void UCombatWidget::ClearContents(const bool bClearAll)
{
	RightVBox->ClearChildren();
	LeftVBox->ClearChildren();

	if (bClearAll) 
	{
		EnemyIconHBox->ClearChildren();
		SelectedCharacterAimingInfo.Empty();
		CenterActionHBox->ClearChildren();
	}
}

void UCombatWidget::Renew(const TArray<FAimingInfo>& AimingInfoArray, const FPossibleActionWrapper& PossibleActionMapWrapper)
{
	ConstructWidgetMinimum();
	ClearContents(true);
	
	SelectedCharacterAimingInfo = AimingInfoArray;
	PossibleActionMap = PossibleActionMapWrapper.PossibleAction;

	//Fill Side Contents 
	if (SelectedCharacterAimingInfo.Num() != 0) 
	{
		FillAimingInfo(SelectedCharacterAimingInfo[0]);
		FillCriticalShotInfo(SelectedCharacterAimingInfo[0]);
	}

	// 상단 Enemy Icon
	FillEnemyList();
	FillActionButtonList();
}

void UCombatWidget::FillAimingInfo(const FAimingInfo& AimingInfo) 
{
	float SumOfAimingPercentage = 0;
	FString Explanation;
	float Percentage;
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
		SumOfAimingPercentage += Percentage;

		FillLeftContnents(Explanation, Percentage);
	}
	UTextBlock* PercentageText = Cast<UTextBlock>(SumAimingProbBox->GetWidgetFromName(FName("PercentageValue")));
	PercentageText->SetText(FText::AsNumber((int8)SumOfAimingPercentage));	
}

void UCombatWidget::FillCriticalShotInfo(const FAimingInfo& AimingInfo) 
{
	float SumOfCriticalPercentage = 0;
	FString Explanation;
	float Percentage;
	for (auto SingleCriticalFactor : AimingInfo.CriticalFactor)
	{
		switch (SingleCriticalFactor.Key)
		{
		case ECriticalFactor::SideAttack:
			Explanation = L"측면 목표";
			break;
		case ECriticalFactor::WeaponAbility:
			Explanation = L"무기 치명타";
			break;
		default:
			break;
		}
		Percentage = SingleCriticalFactor.Value * 100;
		SumOfCriticalPercentage += Percentage;

		FillRightContents(Explanation, Percentage);
	}
	UTextBlock* PercentageText = Cast<UTextBlock>(SumCriticalProbBox->GetWidgetFromName(FName("PercentageValue")));
	PercentageText->SetText(FText::AsNumber(SumOfCriticalPercentage));
}

void UCombatWidget::FillLeftContnents(const FString& Explanation, const float Percentage)
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

//Refactoring 가능
void UCombatWidget::FillRightContents(const FString& Explanation, const float Percentage) 
{
	if (SideContentBoxBlueprint)
	{
		UUserWidget* RightContentsBox = CreateWidget<UUserWidget>(GetWorld(), SideContentBoxBlueprint.Get());
		UTextBlock* CommentText = Cast<UTextBlock>(RightContentsBox->GetWidgetFromName(FName("Comment")));
		UTextBlock* PercentageText = Cast<UTextBlock>(RightContentsBox->GetWidgetFromName(FName("PercentageValue")));
		CommentText->SetText(FText::FromString(Explanation));
		PercentageText->SetText(FText::AsNumber(Percentage));

		RightVBox->AddChild(RightContentsBox);
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
	if (LeftFrame->GetVisibility() != ESlateVisibility::Visible) 
	{
		ConstructWidgetRequiredForAttack();
	}
	ClearContents();

	FillAimingInfo(SelectedCharacterAimingInfo[ButtonIndex]);
	FillCriticalShotInfo(SelectedCharacterAimingInfo[ButtonIndex]);

	ChangeViewTargetDelegate.Execute(SelectedCharacterAimingInfo[ButtonIndex].TargetActor, true, ButtonIndex);
	TargetEnemyIndex = ButtonIndex;
}

void UCombatWidget::FillActionButtonList() 
{
	for (auto SinglePossibleAction : PossibleActionMap) 
	{

		if (SinglePossibleAction.Value == true)
		{
			UCustomButton* ButtonForMarkingEnemy = NewObject<UCustomButton>(UCustomButton::StaticClass());;

			FString ButtonImagePath;
			FName ButtonClickFunctionName;
			switch (SinglePossibleAction.Key)
			{
			case EAction::Attack:
				if (SelectedCharacterAimingInfo.Num() == 0) // 공격 상대 없을경우 버튼생성 생성 건너뜀
				{
					continue;
				}
				ButtonImagePath = "/Game/Texture/NormalAttack.NormalAttack";
				ButtonClickFunctionName = L"AttackButtonClicked";
				break;
			case EAction::Ambush:
				ButtonImagePath = "/Game/Texture/NormalAmbush.NormalAmbush";
				ButtonClickFunctionName = L"AmbushButtonClicked";
				break;
			case EAction::Grenade:
				ButtonImagePath = "/Game/Texture/NormalGrenade.NormalGrenade";
				ButtonClickFunctionName = L"GrenadeButtonClicked";
				break;
			case EAction::Vigilance:
				ButtonImagePath = "/Game/Texture/NormalVigilance.NormalVigilance";
				ButtonClickFunctionName = L"VisilianceButtonClicked";
				break;
			case EAction::None:
				ButtonImagePath = "/Game/Texture/EnemyIcon.EnemyIcon";
				ButtonClickFunctionName = L"AttackButtonClicked";
				break;
			}
			
			TScriptDelegate<FWeakObjectPtr> delegateFunction;
			delegateFunction.BindUFunction(this, ButtonClickFunctionName);
			ButtonForMarkingEnemy->OnClicked.Add(delegateFunction);
			
			UTexture2D* Texture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), NULL, *(ButtonImagePath)));
			
			FSlateBrush TempBrush;
			TempBrush.SetResourceObject(Texture);
			TempBrush.DrawAs = ESlateBrushDrawType::Image;

			FButtonStyle ButtonStyle;
			ButtonStyle.SetNormal(TempBrush);
			ButtonForMarkingEnemy->SetStyle(ButtonStyle);
			
			UHorizontalBoxSlot* HBoxSlot = CenterActionHBox->AddChildToHorizontalBox(ButtonForMarkingEnemy);
			HBoxSlot->SetPadding(FMargin(0, 0, 5, 0));
		}
	}
}

void UCombatWidget::AttackButtonClicked() 
{
	ConstructWidgetRequiredForAttack();
	EnemyButtonClicked(0);

	MainStartActionButton->OnClicked.Clear();

	TScriptDelegate<FWeakObjectPtr> delegateFunction;
	delegateFunction.BindUFunction(this, "StartAttackButtonClicked");
	MainStartActionButton->OnClicked.Add(delegateFunction);
	UTextBlock* StartActionButtonText = Cast<UTextBlock>(MainStartActionButton->GetChildAt(0));
	if (StartActionButtonText) 
	{
		StartActionButtonText->SetText(FText::FromString(L"무기 발사"));
	}
}


void UCombatWidget::GrenadeButtonClicked()
{
	ConstructWidgetNormal();

	MainStartActionButton->OnClicked.Clear();

	if (StartTrajectoryDelegate.IsBound())
	{
		StartTrajectoryDelegate.Execute();
	}
/*
	TScriptDelegate<FWeakObjectPtr> delegateFunction;
	delegateFunction.BindUFunction(this, "StartAttackButtonClicked");
	MainStartActionButton->OnClicked.Add(delegateFunction);
*/
	UTextBlock* StartActionButtonText = Cast<UTextBlock>(MainStartActionButton->GetChildAt(0));
	if (StartActionButtonText)
	{
		StartActionButtonText->SetText(FText::FromString(L"수류탄 투척"));
	}
}

void UCombatWidget::VisilianceButtonClicked()
{
	ConstructWidgetNormal();

	MainStartActionButton->OnClicked.Clear();

	TScriptDelegate<FWeakObjectPtr> delegateFunction;
	delegateFunction.BindUFunction(this, "StartVigilanceButtonClicked");
	MainStartActionButton->OnClicked.Add(delegateFunction);
	UTextBlock* StartActionButtonText = Cast<UTextBlock>(MainStartActionButton->GetChildAt(0));
	if (StartActionButtonText)
	{
		StartActionButtonText->SetText(FText::FromString(L"경계 시작"));
	}	
}

void UCombatWidget::AmbushButtonClicked()
{
	ConstructWidgetNormal();
	UTextBlock* StartActionButtonText = Cast<UTextBlock>(MainStartActionButton->GetChildAt(0));
	if (StartActionButtonText)
	{
		StartActionButtonText->SetText(FText::FromString(L"잠복 시작"));
	}
}

void UCombatWidget::StartAttackButtonClicked() 
{
	UE_LOG(LogTemp, Warning, L"공격 시작");
	if (StartAttackDelegate.IsBound())
	{
		StartAttackDelegate.Execute(TargetEnemyIndex);
	}
}

void UCombatWidget::StartVigilanceButtonClicked()
{
	UE_LOG(LogTemp, Warning, L"경계 시작");

	if (StartVisilianceDelegate.IsBound())
	{
		StartVisilianceDelegate.Execute();
		UE_LOG(LogTemp, Warning, L"경계 시작2");
	}
}

void UCombatWidget::StartAmbushButtonClicked() 
{
	UE_LOG(LogTemp, Warning, L"은신 시작");
	if (StartAmbushDelegate.IsBound())
	{
		StartAmbushDelegate.Execute();
	}
}

void UCombatWidget::StartGrenadeButtonClicked()
{
	UE_LOG(LogTemp, Warning, L"수류탄 조정 시작");
}

void UCombatWidget::ConstructWidgetMinimum()
{
	LeftFrame->SetVisibility(ESlateVisibility::Collapsed);
	RightFrame->SetVisibility(ESlateVisibility::Collapsed);
	CenterFrame->SetVisibility(ESlateVisibility::Collapsed);
	EnemyIconHBox->SetVisibility(ESlateVisibility::Collapsed);
	CenterActionHBox->SetVisibility(ESlateVisibility::Visible);

	PlayAnimationByName(FName("MinimumUIAnim"));
	PlayerController->ShowTileIdicator();
}

void UCombatWidget::ConstructWidgetRequiredForAttack()
{
	LeftFrame->SetVisibility(ESlateVisibility::Visible);
	RightFrame->SetVisibility(ESlateVisibility::Visible);
	CenterFrame->SetVisibility(ESlateVisibility::Visible);
	EnemyIconHBox->SetVisibility(ESlateVisibility::Visible);
	CenterActionHBox->SetVisibility(ESlateVisibility::Visible);

	PlayAnimationByName(FName("AttackUIAnim"));
	PlayerController->HideTileIdicator();
}

void UCombatWidget::ConstructWidgetNormal()
{
	LeftFrame->SetVisibility(ESlateVisibility::Collapsed);
	RightFrame->SetVisibility(ESlateVisibility::Collapsed);
	CenterFrame->SetVisibility(ESlateVisibility::Visible);
	EnemyIconHBox->SetVisibility(ESlateVisibility::Visible);
	CenterActionHBox->SetVisibility(ESlateVisibility::Visible);

	PlayAnimationByName(FName("NormalUIAnim"));
	PlayerController->HideTileIdicator();
}


void UCombatWidget::HideAllWidget()
{
	LeftFrame->SetVisibility(ESlateVisibility::Collapsed);
	RightFrame->SetVisibility(ESlateVisibility::Collapsed);
	CenterFrame->SetVisibility(ESlateVisibility::Collapsed);
	EnemyIconHBox->SetVisibility(ESlateVisibility::Collapsed);
	CenterActionHBox->SetVisibility(ESlateVisibility::Collapsed);

	PlayerController->HideTileIdicator();

}
