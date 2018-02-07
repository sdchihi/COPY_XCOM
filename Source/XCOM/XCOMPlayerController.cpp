// Fill out your copyright notice in the Description page of Project Settings.

#include "XCOMPlayerController.h"
#include "CustomThirdPerson.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "TileManager2.h"

AXCOMPlayerController::AXCOMPlayerController() 
{
}

void AXCOMPlayerController::BeginPlay()
{
	Super::BeginPlay();
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATileManager2::StaticClass(), FoundActors);
	TileManager = Cast<ATileManager2>(FoundActors[0]);

	// 문제없이 수행됨
	//UE_LOG(LogTemp, Warning, L"%s", *TileManager->GetName());
};



void AXCOMPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


};

void AXCOMPlayerController::SetupInputComponent() {
	Super::SetupInputComponent();
	this->InputComponent->BindAction("Click", EInputEvent::IE_Pressed, this, &AXCOMPlayerController::OnClick);

}



void AXCOMPlayerController::OnClick()
{
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

void AXCOMPlayerController::SwitchCharacter(ACustomThirdPerson* TargetCharacter) 
{
	Possess(TargetCharacter);
	EnableInput(this);

	//Change active tiles
	TArray<AActor*> OverlappedTile;
	TargetCharacter->GetOverlappingActors(OverlappedTile);
	//여기까지 해서 액터넘겨주자;
	TArray<AActor*> TilesInRange;
	TileManager->GetNearbyTiles(OverlappedTile[0], 2, TilesInRange);
	
	for (AActor* Tile : TilesInRange) {
		UStaticMeshComponent* TileMesh =Cast<UStaticMeshComponent>(Tile->GetRootComponent());
		TileMesh->SetVisibility(true);


		UE_LOG(LogTemp, Warning, L"Output : %s", *Tile->GetName());
	}
	//UE_LOG(LogTemp, Warning, L"%s", *TileManager->GetName());

	
}

void AXCOMPlayerController::GetNearbyTiles(TArray<AActor*> Tiles, int32 MovingAbility) 
{
	//TileManager->
}