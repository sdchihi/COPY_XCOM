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

//���� ��� ����
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
			{	//�� Ŭ����			(�׽�Ʈ�� �ڵ�- ���Ŀ� �ű���̶� �Լ�ȭ ���� �ʴ´�.)
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
			
			//�ൿ�� �Һ�
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
			//TODO Ÿ�ϰ� ĳ���Ͱ� �ƴ� obj Ŭ���� ó��
		}
	}
}

/**
* ���� �� �ȿ��� ���� ���õǾ� �ִ� ĳ���Ͱ� �ƴ� �ٸ� ĳ���ͷ� ��ȯ�ɶ� ȣ��˴ϴ�.
* @param TargetCharacter - ��ȯ�� ĳ����
*/
void AXCOMPlayerController::SwitchCharacter(ACustomThirdPerson* TargetCharacter)
{
	if (TargetCharacter->RemainingActionPoint > 0) {
		DisableInput(this);

		TArray<AActor*> OverlappedTile;
		TargetCharacter->GetOverlappingActors(OverlappedTile);
		if (OverlappedTile.Num() == 0)		//����ó�� 
		{
			EnableInput(this);
			return;
		}
		FTimerHandle UnUsedHandle;
		FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &AXCOMPlayerController::SetTilesToUseSelectedChararacter, Cast<ATile>(OverlappedTile[0]), SelectedCharacter->CurrentMovableStep, SelectedCharacter->GetMovableStepPerActionPoint());
		GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 0.5f, false);

		//Ŭ���� Actor �̵� �ʿ�   ( ī�޶� �̵��� �ƴ� )
		SelectedCharacter = TargetCharacter;
	}
	else
	{
		//�ൿ Ƚ�� 0 �϶�
	}
}

/**
* �ٸ� ĳ���͸� Ŭ���� ��ȯ������ ȣ��˴ϴ�.
* @param OverlappedTile - ���õ� ĳ���Ͱ� �ö��ִ� Ÿ��
* @param MovingAbility - �̵� ������ ĭ ��
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
* ��ǥ Ÿ�Ϸ� �̵��Ҷ� �ܰ躰�� �̵��ϰ� �մϴ�
* @param Target - ��ǥ �̵� ����
* @param CurrentIndex - ���� ��ġ�� Ÿ���� �ε���
*/
void AXCOMPlayerController::MovingStepByStep(const Path Target, const int32 CurrentIndex)
{
	FVector TargetLocation = TileManager->ConvertIndexToVector(Target.OnTheWay[CurrentIndex]);
	APawnController* PawnController = Cast<APawnController>(SelectedCharacter->GetController());
	PawnController->MoveToLocation(TargetLocation + FVector(-50,-50,0), 0, false, false, false);	// �Ϻ� ����

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
		GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 0.5, false);	// 0.5 Delay ����
	}
}

/**
* ��ǥ�� �ϴ� Ÿ�Ϸ� ĳ������ ���� ���¿� ���� ĳ���Ͱ� �ٸ��� �����Դϴ�.
* @param TargetTileIndex - �̵��� Ÿ���� �ε���
*/
void AXCOMPlayerController::MoveCharacterBasedOnState(int32 TargetTileIndex)
{
	DisableInput(this);
	
	if (SelectedCharacter->bIsCovering)
	{
		SelectedCharacter->ClearCoverDirectionInfo();
		FTimerHandle UnUsedHandle;
		FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &AXCOMPlayerController::MovingStepByStep, TileManager->PathArr[TargetTileIndex], TileManager->PathArr[TargetTileIndex].OnTheWay.Num() - 1);
		GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 1.2, false);	// 0.5 Delay ����
	}
	else
	{
		MovingStepByStep(TileManager->PathArr[TargetTileIndex], TileManager->PathArr[TargetTileIndex].OnTheWay.Num() - 1);
	}
}

/**
* ĳ������ �ֺ��� Wall�� ��ġ�ߴ��� Ȯ���մϴ�.
* ��������Ʈ�� ���������Ұ����� ����
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
* Cadinal ���⿡ ���ؼ� ���� �ִ��� Ȯ���մϴ�.
* @param CharacterIndex - ĳ���Ͱ� ��ġ�� Ÿ���� �ε���
* @param CardinalIndex - Cardinal ������ Ÿ�� �ε���
*/
void AXCOMPlayerController::CheckWallAroundOneDirection(const int32 CharacterIndex, const int CardinalIndex)
{
	int32 RowNumber = 0;
	ECoverDirection CoverDirection = ECoverDirection::None;
	if (CardinalIndex == (CharacterIndex + TileManager->GetGridXLength())) 
	{
		CoverDirection = ECoverDirection::North; // ��
		RowNumber = 1;
	}
	else if (CardinalIndex == (CharacterIndex - TileManager->GetGridXLength())) 
	{
		CoverDirection = ECoverDirection::South; // ��
		RowNumber = -1;
	}
	else
	{
		if (CardinalIndex == CharacterIndex + 1)
		{
			CoverDirection = ECoverDirection::East; // ��
		}
		else 
		{
			CoverDirection = ECoverDirection::West;	// ��
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

// �׽�Ʈ�� ���� �޼ҵ�
// ���Ŀ� ���� �Ǵ� ���� �ʿ�
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