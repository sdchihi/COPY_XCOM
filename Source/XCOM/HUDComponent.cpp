// Fill out your copyright notice in the Description page of Project Settings.

#include "HUDComponent.h"
//#include "Runtime/UMG/Public/Components/WidgetComponent.h"
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


void UHUDComponent::HoverHUD(float AxisValue)
{
	//this->GetRelativeTransform().GetLocation()
	FRotator newRotator = GetRelativeTransform().GetRotation().Rotator() + FRotator(0, 2 * AxisValue, 0);
	SetRelativeRotation(newRotator);
	UE_LOG(LogTemp, Warning, L"HUD Rot = %s", *newRotator.ToString());
}

void UHUDComponent::ControlSpringArmLength(float value)
{
	TargetArmLength = value;
}
