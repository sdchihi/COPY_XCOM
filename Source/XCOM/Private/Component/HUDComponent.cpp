// Fill out your copyright notice in the Description page of Project Settings.

#include "HUDComponent.h"
#include "Components/WidgetComponent.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "PlayerPawn.h"


UHUDComponent::UHUDComponent() 
{
	this->TargetArmLength = 300.0f;
	HPBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HP Bar"));
	HPBarWidget->SetupAttachment(this, USpringArmComponent::SocketName);
	HPBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
	HPBarWidget->SetDrawSize(FVector2D(250, 20));

}

void UHUDComponent::BeginPlay() 
{
	Super::BeginPlay();
	this->SetAbsolute(false, true, false);


	if (HUDWidgetBlueprint) {
		HPBarWidget->SetWidgetClass(HUDWidgetBlueprint);
		HPBarWidget->InitWidget();
	}
	else{
		UE_LOG(LogTemp, Warning, L"Please Regist Widget Class");
	}

	APlayerPawn* PlayerPawn =Cast<APlayerPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	if (PlayerPawn) {
		PlayerPawn->RotateUIDelegate.AddUniqueDynamic(this, &UHUDComponent::HoverHUD);
		//Todo
		PlayerPawn->ControlDistanceToUIDelegate.AddUniqueDynamic(this, &UHUDComponent::ControlSpringArmLength);
	}
}

/**
* �÷��̾� ī�޶��� ȸ��������  HUD�� ��ġ�� �����մϴ�.
* @param AxisValue - ���콺 x�� �̵� �Է� ���� �޾ƿɴϴ�.
*/
void UHUDComponent::HoverHUD(const float AxisValue)
{
	FRotator newRotator = GetRelativeTransform().GetRotation().Rotator() + FRotator(0, 2 * AxisValue, 0);
	SetRelativeRotation(newRotator);
}

void UHUDComponent::ControlSpringArmLength(const float Length)
{
	TargetArmLength = Length;
}


void UHUDComponent::SetWidgetVisibility(const bool bVisiblity) 
{
	HPBarWidget->SetVisibility(bVisiblity);
}
