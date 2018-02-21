// Fill out your copyright notice in the Description page of Project Settings.

#include "XCOMPlayerController.h"
#include "CustomThirdPerson.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "TileManager2.h"
#include "Tile.h"
#include "PawnController.h"

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
	GetHitResultUnderCursor(ECollisionChannel::ECC_MAX, true, TraceResult);
	AActor* actor= TraceResult.GetActor();

	if (!ensure(TraceResult.GetActor())) {
		return;
	}
	else {
		ACustomThirdPerson* TargetCharacter = Cast<ACustomThirdPerson>(actor);
		ATile* TargetTile = Cast<ATile>(actor);
		if (TargetCharacter) {
			DisableInput(this);
			SetViewTargetWithBlend(TargetCharacter, 0.5);

			FTimerHandle UnUsedHandle;
			FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &AXCOMPlayerController::SwitchCharacter, TargetCharacter);

			GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 0.5f, false);
			SelectedCharacter = TargetCharacter;


			UE_LOG(LogTemp, Warning, L"Pawn Clicked!");

			UE_LOG(LogTemp, Warning, L"Controller Name  : %s", *SelectedCharacter->GetController()->GetName());

		}
		else if (TargetTile) {
			if (!SelectedCharacter) { 
				UE_LOG(LogTemp, Warning, L"SelectedCharacter invalid!");
				return; 
			}
			//오류 검출

			UE_LOG(LogTemp, Warning, L"%d Tile Clicked!", TileManager->ConvertVectorToIndex(TargetTile->GetActorLocation()));


			APawnController* PawnController = Cast<APawnController>(SelectedCharacter->GetController());
			
			
			int32 TileIndex = TileManager->ConvertVectorToIndex(TargetTile->GetActorLocation());
			UE_LOG(LogTemp, Warning, L"Path Length : %d", TileManager->PathArr[TileIndex].OnTheWay.Num());
			for (int i = TileManager->PathArr[TileIndex].OnTheWay.Num() - 1; i >= 0; i--) {
				
				FVector TargetLocation = TileManager->ConvertIndexToVector(TileManager->PathArr[TileIndex].OnTheWay[i]);
				//UE_LOG(LogTemp, Warning, L"이 타일로 움직입니다 : %d", PathIndex);

				PawnController->MoveToTargetLocation(TargetLocation);
				FTimerHandle UnusedHandle;
				GetWorldTimerManager().SetTimer(UnusedHandle, 2.5, false);

				UE_LOG(LogTemp, Warning, L"한번");

				//여기
			}

			//if (PawnController) {
			//	int32 PathIndex = TileManager->PathArr[TileIndex].OnTheWay[0];
			//	FVector TargetLocation = TileManager->ConvertIndexToVector(PathIndex);
			//	PawnController->MoveToTargetLocation(TargetLocation);
			//	
			//	UE_LOG(LogTemp, Warning, L"TargetIndex : %d     Target Location : %s    ", PathIndex, *TargetLocation.ToString());
			//}
			//else {
			//	UE_LOG(LogTemp, Warning, L"Fxxxx");
			//}

			
			for (int i = 0; i < TileManager->PathArr[TileIndex].OnTheWay.Num(); i++) {
				UE_LOG(LogTemp, Warning, L"%d", TileManager->PathArr[TileIndex].OnTheWay[i]);

			}

		}
		else {
			UE_LOG(LogTemp, Warning, L"%s", *actor->GetName());
		}
	}
}

void AXCOMPlayerController::SwitchCharacter(ACustomThirdPerson* TargetCharacter) 
{

	//Possess(TargetCharacter);
	EnableInput(this);

	TileManager->ClearAllTiles(true);

	TArray<AActor*> OverlappedTile;
	TargetCharacter->GetOverlappingActors(OverlappedTile);

	if (OverlappedTile.Num() == 0) { return; }

	TArray<ATile*> TilesInRange;
	TileManager->GetAvailableTiles(Cast<ATile>(OverlappedTile[0]), 4, TilesInRange);


	for (ATile* Tile : TilesInRange) {
		UStaticMeshComponent* TileMesh =Cast<UStaticMeshComponent>(Tile->GetRootComponent());
		TileMesh->SetVisibility(true);
	}

	//ACharacter::MoveT

}

