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
#include "PlayerPawn.h"
#include "PlayerPawnInAiming.h"

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

	Initialize();
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

void AXCOMPlayerController::Initialize() {
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATileManager2::StaticClass(), FoundActors);
	TileManager = Cast<ATileManager2>(FoundActors[0]);

	FoundActors.Empty();
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerPawnInAiming::StaticClass(), FoundActors);
	PawnInAimingSituation = Cast<APlayerPawnInAiming>(FoundActors[0]);

	FoundActors.Empty();
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerPawn::StaticClass(), FoundActors);
	DefaultPlayerPawn = Cast<APlayerPawn>(FoundActors[0]);

	FoundActors.Empty();
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACustomThirdPerson::StaticClass(), FoundActors);
	for (auto ThirdPersonAsActor : FoundActors) 
	{
		ACustomThirdPerson* SingleThirdPerson = Cast<ACustomThirdPerson>(ThirdPersonAsActor);
		SingleThirdPerson->ChangePlayerPawnDelegate.BindDynamic(this, &AXCOMPlayerController::ChangeToDefaultPawn);


		if (SingleThirdPerson->GetTeamFlag()) 
		{
			PlayerCharacters.Add(SingleThirdPerson);
		}
	}
}

void AXCOMPlayerController::OnClick()
{
	FHitResult TraceResult;
	GetHitResultUnderCursor(ECollisionChannel::ECC_MAX, true, TraceResult);
	AActor* actor = TraceResult.GetActor();

	if (!ensure(TraceResult.GetActor())) { return; }
	else
	{
		ACustomThirdPerson* TargetCharacter = Cast<ACustomThirdPerson>(actor);
		ATile* TargetTile = Cast<ATile>(actor);
		if (TargetCharacter)
		{
			if (CheckClickedCharacterTeam(TargetCharacter))
			{
				SwitchCharacter(TargetCharacter);
			}
			else
			{	//적 클릭시			(테스트용 코드- 이후에 옮길것이라서 함수화 하지 않는다.)
				PawnInAimingSituation->SetCameraPositionInAimingSituation(SelectedCharacter->GetActorLocation(), TargetCharacter->GetActorLocation());
				TileManager->ClearAllTiles(true);
				SetViewTargetWithBlend(PawnInAimingSituation, 0.5);
				//SelectedCharacter->CheckAttackPotential(TargetCharacter);
			}
		}
		else if (TargetTile)
		{
			if (!IsValid(SelectedCharacter))
			{
				UE_LOG(LogTemp, Warning, L"SelectedCharacter invalid!");
				return;
			}

			if (TargetTile->GetTileVisibility() == false)
			{
				UE_LOG(LogTemp, Warning, L"Can not be moved there");
				return;
			}
			
			//행동력 소비
			if(TargetTile->bCanMoveWithOneAct) 
			{
				SelectedCharacter->UseActionPoint(1);
				SelectedCharacter->CurrentMovableStep /= 2;
			}
			else 
			{
				SelectedCharacter->UseActionPoint(2);
			}
			int32 TargetTileIndex = TileManager->ConvertVectorToIndex(TargetTile->GetActorLocation());
			MoveCharacterBasedOnState(TargetTileIndex);
		}
		else
		{
			//TODO 타일과 캐릭터가 아닌 obj 클릭시 처리
		}
	}
}

/**
* 같은 편 안에서 현재 선택되어 있는 캐릭터가 아닌 다른 캐릭터로 전환될때 호출됩니다.
* @param TargetCharacter - 전환할 캐릭터
*/
void AXCOMPlayerController::SwitchCharacter(ACustomThirdPerson* TargetCharacter)
{
	if (TargetCharacter->RemainingActionPoint > 0) {
		DisableInput(this);

		TArray<AActor*> OverlappedTile;
		TargetCharacter->GetOverlappingActors(OverlappedTile);
		if (OverlappedTile.Num() == 0)		//예외처리 
		{
			EnableInput(this);
			return;
		}
		FTimerHandle UnUsedHandle;
		FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &AXCOMPlayerController::SetTilesToUseSelectedChararacter, Cast<ATile>(OverlappedTile[0]), SelectedCharacter->CurrentMovableStep, SelectedCharacter->GetMovableStepPerActionPoint());
		GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 0.5f, false);

		//클릭시 Actor 이동 필요   ( 카메라 이동은 아님 )
		SelectedCharacter = TargetCharacter;
	}
	else
	{
		//행동 횟수 0 일때
	}
}

/**
* 다른 캐릭터를 클릭해 전환됬을때 호출됩니다.
* @param OverlappedTile - 선택된 캐릭터가 올라가있는 타일
* @param MovingAbility - 이동 가능한 칸 수
*/
void AXCOMPlayerController::SetTilesToUseSelectedChararacter(ATile* OverlappedTile, const int32 MovingAbility,const int32 MovableStepPerAct)
{
	EnableInput(this);
	TileManager->ClearAllTiles(true);

	TArray<ATile*> TilesInRange;
	TileManager->GetAvailableTiles(OverlappedTile, MovingAbility, MovableStepPerAct, TilesInRange);

	for (ATile* Tile : TilesInRange)
	{
		UStaticMeshComponent* TileMesh = Cast<UStaticMeshComponent>(Tile->GetRootComponent());
		if (Tile->bCanMoveWithOneAct) 
		{
			Tile->SelectCloseTileMaterial();
		}
		else 
		{
			Tile->SelectDistantTileMaterial();
		}

		TileMesh->SetVisibility(true);
	}
}

/**
* 목표 타일로 이동할때 단계별로 이동하게 합니다
* @param Target - 목표 이동 지점
* @param CurrentIndex - 현재 위치한 타일의 인덱스
*/
void AXCOMPlayerController::MovingStepByStep(const Path Target, const int32 CurrentIndex)
{
	FVector TargetLocation = TileManager->ConvertIndexToVector(Target.OnTheWay[CurrentIndex]);
	APawnController* PawnController = Cast<APawnController>(SelectedCharacter->GetController());
	PawnController->MoveToLocation(TargetLocation + FVector(-50,-50,0), 0, false, false, false);	// 일부 보정

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

/**
* 목표로 하는 타일로 캐릭터의 엄폐 상태에 따라 캐릭터가 다르게 움직입니다.
* @param TargetTileIndex - 이동할 타일의 인덱스
*/
void AXCOMPlayerController::MoveCharacterBasedOnState(int32 TargetTileIndex)
{
	DisableInput(this);
	
	if (SelectedCharacter->bIsCovering)
	{
		SelectedCharacter->ClearCoverDirectionInfo();
		FTimerHandle UnUsedHandle;
		FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &AXCOMPlayerController::MovingStepByStep, TileManager->PathArr[TargetTileIndex], TileManager->PathArr[TargetTileIndex].OnTheWay.Num() - 1);
		GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 1.2, false);	// 0.5 Delay 고정
	}
	else
	{
		MovingStepByStep(TileManager->PathArr[TargetTileIndex], TileManager->PathArr[TargetTileIndex].OnTheWay.Num() - 1);
	}
}

/**
* 캐릭터의 주변에 Wall이 위치했는지 확인합니다.
* 델리게이트로 수정가능할것으로 보임
*/
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

	if (SelectedCharacter->bIsCovering) 
	{
		SelectedCharacter->RotateTowardWall();
	}

	//Todo
	SelectedCharacter->ScanEnemy();
}

/**
* Cadinal 방향에 대해서 벽이 있는지 확인합니다.
* @param CharacterIndex - 캐릭터가 위치한 타일의 인덱스
* @param CardinalIndex - Cardinal 방향의 타일 인덱스
*/
void AXCOMPlayerController::CheckWallAroundOneDirection(const int32 CharacterIndex, const int CardinalIndex)
{
	int32 RowNumber = 0;
	ECoverDirection CoverDirection = ECoverDirection::None;
	if (CardinalIndex == (CharacterIndex + TileManager->GetGridXLength())) 
	{
		CoverDirection = ECoverDirection::North; // 북
		RowNumber = 1;
	}
	else if (CardinalIndex == (CharacterIndex - TileManager->GetGridXLength())) 
	{
		CoverDirection = ECoverDirection::South; // 남
		RowNumber = -1;
	}
	else
	{
		if (CardinalIndex == CharacterIndex + 1)
		{
			CoverDirection = ECoverDirection::East; // 동
		}
		else 
		{
			CoverDirection = ECoverDirection::West;	// 서
		}
		RowNumber = 0;
	}

	if (TileManager->CheckWithinBounds(CardinalIndex) && TileManager->IsSameLine(CharacterIndex, RowNumber, CardinalIndex) &&
		TileManager->PathArr[CardinalIndex].bWall) 
	{
		SelectedCharacter->CoverDirection = CoverDirection;
		SelectedCharacter->CoverDirectionMap.Add(CoverDirection, TileManager->PathArr[CardinalIndex].CoverInfo);
		SelectedCharacter->bIsCovering = true;
	}
}

// 테스트를 위한 메소드
// 이후에 삭제 또는 수정 필요
bool AXCOMPlayerController::CheckClickedCharacterTeam(ACustomThirdPerson* ClickedCharacter) 
{
	if (!SelectedCharacter) 
	{ 
		SelectedCharacter = ClickedCharacter;
		return true; 
	}

	if (SelectedCharacter->GetTeamFlag()  == ClickedCharacter->GetTeamFlag())
	{
		return true;
	}
	else if(!(SelectedCharacter->bCanAction))
	{
		return true;
	}
	return false;
}

void AXCOMPlayerController::ChangeToDefaultPawn() {
	SetViewTargetWithBlend(DefaultPlayerPawn, 0.5);
};

FVector AXCOMPlayerController::GetNextAvailableCharLocation() 
{
	TArray<ACustomThirdPerson*> AvailableCharacters;
	for (auto SinglePlayerChar : PlayerCharacters)
	{
		if (SinglePlayerChar->bCanAction) 
		{
			AvailableCharacters.Add(SinglePlayerChar);
		}
	}
	int32 NextIndex = CharacterSwitchIndex % AvailableCharacters.Num();
	CharacterSwitchIndex++;

	return AvailableCharacters[NextIndex]->GetActorLocation();;
}

void AXCOMPlayerController::ReleaseCharacter() 
{
	TileManager->ClearAllTiles(true);
	//TODO UI OFF
	
	
}