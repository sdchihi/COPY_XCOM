// Fill out your copyright notice in the Description page of Project Settings.

#include "XCOMPlayerController.h"
#include "CustomThirdPerson.h"

AXCOMPlayerController::AXCOMPlayerController() 
{
	bShowMouseCursor = true;
	ShouldShowMouseCursor();
}

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
		ACustomThirdPerson* character = Cast<ACustomThirdPerson>(actor);
		if (!ensure(character)) { return; }
		else {
			//Possess(character);
		}

		UE_LOG(LogTemp, Warning, L"Actor Clicked!!");
	}
}
