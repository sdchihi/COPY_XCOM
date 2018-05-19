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

	/*for (auto temp : MovableTiles) 
	{
		int32 Index =TileManager->ConvertVectorToIndex(temp->GetActorLocation());
		UE_LOG(LogTemp, Warning, L"AI  이동 가능한 타일 index : %d", Index);
	}*/




	TMap<ATile*, FAICommandInfo> TileScoreBoard = GetScoreBoard(MovableTiles);
	int32 size = 0;
	for (auto& temp : TileScoreBoard)
	{
		int32 Index = TileManager->ConvertVectorToIndex(temp.Key->GetActorLocation());
		int32 Score = temp.Value.Score;
		UE_LOG(LogTemp, Warning, L"AI  이동 가능한 타일 index : %d  점수: %d", Index, Score);
	}
	UE_LOG(LogTemp, Warning, L"AI  이동 가능한 타일 수 : %d", size);


	FindBestScoredAction(TileScoreBoard);
}


TMap<ATile*, FAICommandInfo> AEnemyController::GetScoreBoard(TArray<ATile*> MovableTiles)
{
	TMap<ATile*, FAICommandInfo> TileScoreBoard;
	TArray<ACustomThirdPerson*> PlayerCharacters = GetPlayerCharacters();

	for (auto TargeTile : MovableTiles)
	{
		int32 GeographicalScore = 0;
		int32 ActionScore = 0;

		FVector TileLocation = TargeTile->GetActorLocation();
		TArray<FVector> CoverDirectionArr;
		bool bWallAround = TileManager->CheckWallAround(TileLocation, CoverDirectionArr);
		if (bWallAround)	//엄폐 가능.
		{
			for (ACustomThirdPerson* Unit : PlayerCharacters)
			{
				FVector UnitLocation = Unit->GetActorLocation();
				

				if ( !CheckMimiumInterval(TileLocation, UnitLocation))// 최소 간격 유지 실패시 Score - 50 패널티 
				{
					GeographicalScore -= 50;
				}
				if (IsProtectedByCover(TileLocation, UnitLocation, CoverDirectionArr)) 
				{
					GeographicalScore += 20;
				}
			}
			FAimingInfo BestAimingInfo;
			ScoringByAimingInfo(CoverDirectionArr, ActionScore, BestAimingInfo);
			
			EAction ActionOnTargetTile = DecideActionOnTile(ActionScore);

			int32 TotalScore = ActionScore + GeographicalScore;
			FAICommandInfo CommandInfo;// = FAICommandInfo(TotalScore, &BestAimingInfo, ActionOnTargetTile);
			CommandInfo.Action = ActionOnTargetTile;
			CommandInfo.AimingInfo = &BestAimingInfo;
			CommandInfo.Score = TotalScore;

			TileScoreBoard.Add(TargeTile, CommandInfo);
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

void AEnemyController::FindBestScoredAction(const TMap<ATile*, FAICommandInfo> TileScoreBoard)
{
	int32 HighestScore = 0;
	ATile* HighestScoredTile = nullptr;
	for (auto It = TileScoreBoard.CreateConstIterator(); It; ++It)		//읽기 전용 Interator    //읽기 쓰기는 CreateIterator
	{
		if (HighestScore < It.Value().Score) 
		{
			HighestScore = It.Value().Score;
			HighestScoredTile = It.Key();
		}
	}
	int32 TileIndex = TileManager->ConvertVectorToIndex(HighestScoredTile->GetActorLocation());
	UE_LOG(LogTemp, Warning, L"AI탐색 결과 -  TileIndex  : %d  Scroed : %d", TileIndex, HighestScore);
	//블랙보드 세팅

}