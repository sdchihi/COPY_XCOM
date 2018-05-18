// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyController.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "TileManager2.h"
#include "CustomThirdPerson.h"
#include "Tile.h"
#include "AimingComponent.h"

void AEnemyController::BeginPlay()
{
	Super::BeginPlay();
	PrimaryActorTick.bCanEverTick = false;


	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATileManager2::StaticClass(), FoundActors);
	TileManager = Cast<ATileManager2>(FoundActors[0]);

}


void AEnemyController::SetNextAction()
{
	ACustomThirdPerson* ControlledPawn = Cast<ACustomThirdPerson>(GetControlledPawn());
	if (!ControlledPawn) { return; }
	int32 MovableStep = ControlledPawn->GetMovableStepPerActionPoint();


	ATile* OverllapedTile = TileManager->GetOverlappedTile(ControlledPawn);
	if (!OverllapedTile) { return; }

	TArray<ATile*> MovableTiles;
	TileManager->GetAvailableTiles(OverllapedTile, MovableStep, MovableStep, MovableTiles);

	
	TMap<ATile*, int32> TileScoreBoard = GetScoreBoard(MovableTiles);
}


TMap<ATile*, int32> AEnemyController::GetScoreBoard(TArray<ATile*> MovableTiles)
{
	TMap<ATile*, int32> TileScoreBoard;
	TArray<ACustomThirdPerson*> PlayerCharacters = GetPlayerCharacters();

	for (auto TargeTile : MovableTiles)
	{
		int32 Score = 0;
		FVector TileLocation = TargeTile->GetActorLocation();
		TArray<FVector> CoverDirectionArr;
		bool bWallAround = TileManager->CheckWallAround(TileLocation, CoverDirectionArr);
		if (bWallAround)	//���� ����.
		{
			for (ACustomThirdPerson* Unit : PlayerCharacters)
			{
				FVector UnitLocation = Unit->GetActorLocation();
				int32 GeographicalScore = 0;
				int32 ActionScore = 0;

				if ( !CheckMimiumInterval(TileLocation, UnitLocation))// �ּ� ���� ���� ���н� Score - 50 �г�Ƽ 
				{
					GeographicalScore -= 50;
				}
				if (IsProtectedByCover(TileLocation, UnitLocation, CoverDirectionArr)) 
				{
					GeographicalScore += 20;
				}

				FAimingInfo BestAimingInfo;
				ScoringByAimingInfo(CoverDirectionArr, ActionScore, BestAimingInfo);
				



			}
			TileScoreBoard.Add(TargeTile, Score);
		}
	}

	return TileScoreBoard;
}

TArray<ACustomThirdPerson*> AEnemyController::GetPlayerCharacters()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACustomThirdPerson::StaticClass(), FoundActors);
	TArray<ACustomThirdPerson*> PlayerCharacters;

	for (auto SingleActor : FoundActors)
	{
		ACustomThirdPerson* SingleCharacter = Cast<ACustomThirdPerson>(SingleActor);
		if (SingleCharacter->GetTeamFlag())
		{
			PlayerCharacters.Add(SingleCharacter);
		}

	}
	return PlayerCharacters;
}


bool AEnemyController::CheckMimiumInterval(const FVector TileLocation, const FVector TargetActorLocation) 
{
	float Distance = FVector::Dist2D(TileLocation, TargetActorLocation);
	if (Distance <= TileManager->GetTileSize() * 5)	
	{
		return false;
	}
	return true;
}

bool AEnemyController::IsProtectedByCover(const FVector TileLocation, const FVector TargetActorLocation, const TArray<FVector> CoverDirectionArr) 
{
	FVector DirectionToTargetUnit = (TargetActorLocation - TileLocation).GetSafeNormal2D();
	for (FVector CoverDirection : CoverDirectionArr)
	{
		float AngleBtwTargetAndWall = FMath::RadiansToDegrees(acosf(FVector::DotProduct(DirectionToTargetUnit, CoverDirection)));
		AngleBtwTargetAndWall = FMath::Abs(AngleBtwTargetAndWall);
		if (AngleBtwTargetAndWall < 90)
		{
			return true;
		}
	}
	return false;
}




void AEnemyController::ScoringByAimingInfo(TArray<FVector> CoverDirectionArr, OUT int32& ActionScore, OUT FAimingInfo& BestAimingInfo)
{

	TMap<EDirection, ECoverInfo> CoverDirectionMap = MakeCoverDirectionMap(CoverDirectionArr);
	ACustomThirdPerson* ControlledPawn = Cast<ACustomThirdPerson>(GetControlledPawn());
	if (!ControlledPawn) { return; }
	UAimingComponent* AimingComp = ControlledPawn->GetAimingComponent();

	BestAimingInfo = AimingComp->GetBestAimingInfo(ControlledPawn->AttackRadius, ControlledPawn->bIsCovering, CoverDirectionMap);
	int32 BestProb = BestAimingInfo.Probability;

	int32 MinimumAimingProb = 35;
	int32 MaximumAimingProb = 50;	
	if (MaximumAimingProb < BestProb)
	{
		ActionScore = 40;
	}
	else if (BestProb < MinimumAimingProb)
	{
		ActionScore = 20;
	}
	else 
	{
		float LerpAlpha = (BestProb - MinimumAimingProb) / (MaximumAimingProb - MinimumAimingProb); //   x / 15
		ActionScore = FMath::Lerp( 25, 40, LerpAlpha);
	}
}

TMap<EDirection, ECoverInfo> AEnemyController::MakeCoverDirectionMap(TArray<FVector> CoverDirectionArr) 
{
	TMap<EDirection, ECoverInfo> CoverDirectionMap;
	FVector East = FVector(-1, 0, 0);
	FVector West = FVector(1, 0, 0);
	FVector North = FVector(0, 1, 0);
	FVector South = FVector(0, -1, 0);

	EDirection Direction = EDirection::None;
	for (FVector CoverDirection : CoverDirectionArr)
	{
		if (CoverDirection.Equals(East)) { Direction = EDirection::East; }
		else if (CoverDirection.Equals(West)) { Direction = EDirection::West; }
		else if (CoverDirection.Equals(North)) { Direction = EDirection::North; }
		else  { Direction = EDirection::South; }
		CoverDirectionMap.Add(Direction, ECoverInfo::None);
	}
	return CoverDirectionMap;
}

EAction AEnemyController::DecideActionOnTile(int32 ActionScore) 
{
	if (ActionScore == 20)
	{
		return EAction::Vigilance;
	}
	else 
	{
		return EAction::Attack;
	}
}