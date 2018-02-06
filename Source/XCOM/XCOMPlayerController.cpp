// Fill out your copyright notice in the Description page of Project Settings.

#include "XCOMPlayerController.h"
#include "CustomThirdPerson.h"
#include "Runtime/Engine/Public/TimerManager.h"

AXCOMPlayerController::AXCOMPlayerController() 
{
}

void AXCOMPlayerController::BeginPlay()
{
	Super::BeginPlay();
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
};



void AXCOMPlayerController::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);


};

void AXCOMPlayerController::SetupInputComponent() {
	Super::SetupInputComponent();
	this->InputComponent->BindAction("Click", EInputEvent::IE_Pressed, this, &AXCOMPlayerController::OnClick);

}



void AXCOMPlayerController::OnClick() {
	FHitResult TraceResult;
	GetHitResultUnderCursor(ECollisionChannel::ECC_WorldDynamic, false, TraceResult);
	AActor* actor= TraceResult.GetActor();

	if (!ensure(TraceResult.GetActor())) {
		return;
	}
	else {
		ACustomThirdPerson* TargetCharacter = Cast<ACustomThirdPerson>(actor);
		if (IsValid(TargetCharacter)) {
			DisableInput(this);
			SetViewTargetWithBlend(TargetCharacter, 0.5);

			FTimerHandle UnUsedHandle;
			FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &AXCOMPlayerController::SwitchCharacter, TargetCharacter);

			GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 0.5f, false);
		}

	}
}

void AXCOMPlayerController::SwitchCharacter(ACustomThirdPerson* TargetCharacter) {
	Possess(TargetCharacter);
	EnableInput(this);
}