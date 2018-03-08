// Fill out your copyright notice in the Description page of Project Settings.

#include "XCOMPlayerController.h"
#include "CustomThirdPerson.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "TileManager2.h"
#include "Tile.h"
#include "PawnController.h"
#include "Path.h"
#include "Gun.h"

AXCOMPlayerController::AXCOMPlayerController() 
{
}

void AXCOMPlayerController::BeginPlay()
{
	Super::BeginPlay();
	PrimaryActorTick.bCanEverTick = false;


	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATileManager2::StaticClass(), FoundActors);
	TileManager = Cast<ATileManager2>(FoundActors[0]);
};


//아직 사용 안함
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
	UE_LOG(LogTemp, Warning, L"진입123");

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


			if (CheckClickedCharacterTeam(TargetCharacter)) {
				DisableInput(this);

				FTimerHandle UnUsedHandle;
				FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &AXCOMPlayerController::SwitchCharacter, TargetCharacter);


				//클릭시 카메라 위치 변경 효과 차후 수정 필요
				//SetViewTargetWithBlend(TargetCharacter, 0.5);

				GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 0.5f, false);
				SelectedCharacter = TargetCharacter;

			}
			else {//적 클릭시

				SelectedCharacter->CheckAttackPotential(TargetCharacter);
			}

			
		}
		else if (TargetTile) {
			if (!IsValid(SelectedCharacter)) { 
				UE_LOG(LogTemp, Warning, L"SelectedCharacter invalid!");
				return; 
			}
			//UE_LOG(LogTemp, Warning, L"%d Tile Clicked!", TileManager->ConvertVectorToIndex(TargetTile->GetActorLocation()));

			if (TargetTile->GetTileVisibility() == false) {
				UE_LOG(LogTemp, Warning, L"Can not be moved there");
				return;
			}

			int32 TileIndex = TileManager->ConvertVectorToIndex(TargetTile->GetActorLocation());

			DisableInput(this);

			if (SelectedCharacter->bIsCovering) {
				SelectedCharacter->ClearCoverDirectionInfo();
				FTimerHandle UnUsedHandle;
				FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &AXCOMPlayerController::MovingStepByStep, TileManager->PathArr[TileIndex], TileManager->PathArr[TileIndex].OnTheWay.Num() - 1);
				GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 1.2, false);	// 0.5 Delay 고정
			}
			else {
				MovingStepByStep(TileManager->PathArr[TileIndex], TileManager->PathArr[TileIndex].OnTheWay.Num() - 1);
			}
			
			
	

		}
		else {

		}
	}
}

void AXCOMPlayerController::SwitchCharacter(ACustomThirdPerson* TargetCharacter) 
{
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
}

void AXCOMPlayerController::MovingStepByStep(Path Target, int32 CurrentIndex) 
{
	FVector TargetLocation = TileManager->ConvertIndexToVector(Target.OnTheWay[CurrentIndex]);
	APawnController* PawnController = Cast<APawnController>(SelectedCharacter->GetController());
	PawnController->MoveToLocation(TargetLocation + FVector(-50,-50,0), 0, false, false, false);

	UE_LOG(LogTemp, Warning, L"보정 전 좌표: %s , 보정 후 좌표 : %s", *TargetLocation.ToString(), *(TargetLocation + FVector(50, -50, 0)).ToString());

	if (CurrentIndex != 0) 
	{
		// Delegate
		FTimerHandle UnUsedHandle;
		FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &AXCOMPlayerController::MovingStepByStep, Target, CurrentIndex-1);
		GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 0.2, false);
	}
	else if (CurrentIndex == 0) 
	{
		EnableInput(this);
		TileManager->ClearAllTiles(true);
		FTimerHandle UnUsedHandle;
		FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &AXCOMPlayerController::CheckWallAround);
		GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 0.5, false);	// 0.5 Delay 고정
	}
}

void AXCOMPlayerController::CheckWallAround() 
{
	if (!SelectedCharacter) { return; }

	FVector CharacterPos = SelectedCharacter->GetActorLocation();
	int32 CharacterTileIndex = TileManager->ConvertVectorToIndex(CharacterPos);
	UE_LOG(LogTemp, Warning, L"Move To : %d", CharacterTileIndex)

	int32 EastIndex = CharacterTileIndex + 1;
	int32 WestIndex = CharacterTileIndex - 1;
	int32 SouthIndex = CharacterTileIndex - TileManager->GetGridXLength();
	int32 NorthIndex = CharacterTileIndex + TileManager->GetGridXLength();

	CheckWallAroundOneDirection(CharacterTileIndex, EastIndex);
	CheckWallAroundOneDirection(CharacterTileIndex, SouthIndex);
	CheckWallAroundOneDirection(CharacterTileIndex, NorthIndex);
	CheckWallAroundOneDirection(CharacterTileIndex, WestIndex);

	if (SelectedCharacter->bIsCovering) {
		SelectedCharacter->RotateTowardWall();
	}
}

void AXCOMPlayerController::CheckWallAroundOneDirection(int32 CharacterIndex, int CardinalIndex)
{
	int32 RowNumber = 0;
	ECoverDirection CoverDirection = ECoverDirection::None;
	if (CardinalIndex == (CharacterIndex + TileManager->GetGridXLength())) {
		CoverDirection = ECoverDirection::North; // 북
		RowNumber = 1;
	}
	else if (CardinalIndex == (CharacterIndex - TileManager->GetGridXLength())) {
		CoverDirection = ECoverDirection::South; // 남
		RowNumber = -1;
	}
	else {
		if (CardinalIndex == CharacterIndex + 1) {
			CoverDirection = ECoverDirection::East; // 동
		}
		else {
			CoverDirection = ECoverDirection::West;	// 서
		}
		RowNumber = 0;
	}
	UE_LOG(LogTemp, Warning, L"^^2 char:  %d,  cardinal:  %d    row n : %d", CharacterIndex, CardinalIndex, RowNumber)

	UE_LOG(LogTemp, Warning, L"^^2 wall : %d  same line : %d", TileManager->PathArr[CardinalIndex].bWall, TileManager->IsSameLine(CharacterIndex, RowNumber, CardinalIndex))

	if (TileManager->CheckWithinBounds(CardinalIndex) && TileManager->IsSameLine(CharacterIndex, RowNumber, CardinalIndex) &&
		TileManager->PathArr[CardinalIndex].bWall) {
		SelectedCharacter->CoverDirection = CoverDirection;
		SelectedCharacter->CoverDirectionMap.Add(CoverDirection, true);
		SelectedCharacter->bIsCovering = true;
		UE_LOG(LogTemp,Warning, L"^^@@@@@")
	}
}


// 테스트를 위한 메소드
// 이후에 삭제 또는 수정 필요
bool AXCOMPlayerController::CheckClickedCharacterTeam(ACustomThirdPerson* ClickedCharacter) 
{
	if (!SelectedCharacter) { 
		SelectedCharacter = ClickedCharacter;
		return true; 
	}

	if (SelectedCharacter->GetTeamFlag()  == ClickedCharacter->GetTeamFlag()) {
		return true;
	}
	else if(!(SelectedCharacter->bCanAction)){
		return true;
	}

	return false;
}