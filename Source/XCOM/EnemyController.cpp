// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyController.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "TileManager2.h"
#include "CustomThirdPerson.h"
#include "Tile.h"
#include "AimingComponent.h"
#include "Kismet/KismetSystemLibrary.h"

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
	//int32 size = 0;
	//for (auto& temp : TileScoreBoard)
	//{
	//	int32 Index = TileManager->ConvertVectorToIndex(temp.Key->GetActorLocation());
	//	int32 Score = temp.Value.Score;
	//	UE_LOG(LogTemp, Warning, L"AI  이동 가능한 타일 index : %d  점수: %d", Index, Score);
	//	size++;
	//}
	//UE_LOG(LogTemp, Warning, L"AI  이동 가능한 타일 수 : %d", size);


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
					int32 TileIndex = TileManager->ConvertVectorToIndex(TileLocation);
					if (TileIndex == 168) 
					{
						UE_LOG(LogTemp, Warning, L"보호됨")
						UKismetSystemLibrary::DrawDebugPoint(
							GetWorld(),
							UnitLocation + FVector(0, 0, 100),
							20,
							FColor(255,255,0),
							20
						);
					}

					GeographicalScore += 20;
				}
			}
			FAimingInfo BestAimingInfo;
			ScoringByAimingInfo(TileLocation, CoverDirectionArr, ActionScore, BestAimingInfo);
			
			EAction ActionOnTargetTile = DecideActionOnTile(ActionScore);

			int32 TotalScore = ActionScore + GeographicalScore;
			FAICommandInfo CommandInfo;// = FAICommandInfo(TotalScore, &BestAimingInfo, ActionOnTargetTile);
			CommandInfo.Action = ActionOnTargetTile;
			CommandInfo.AimingInfo = &BestAimingInfo;
			CommandInfo.Score = TotalScore;

			TileScoreBoard.Add(TargeTile, CommandInfo);

			int32 Index = TileManager->ConvertVectorToIndex(TargeTile->GetActorLocation());
			UE_LOG(LogTemp, Warning, L"AI  이동 가능한 타일 index : %d  점수: (지리 점수)%d + (액션 점수)%d = %d", Index, GeographicalScore, ActionScore, TotalScore);

			DebugAimingInfo(TileLocation, TotalScore);
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
		if (AngleBtwTargetAndWall < 89)
		{
			return true;
		}
	}
	return false;
}




void AEnemyController::ScoringByAimingInfo(const FVector TileLocation, TArray<FVector> CoverDirectionArr, OUT int32& ActionScore, OUT FAimingInfo& BestAimingInfo)
{
	float PawnLocationZ = GetPawn()->GetTargetLocation().Z;
	FVector ActorOnTileLocation = FVector(TileLocation.X, TileLocation.Y, PawnLocationZ);
	TMap<EDirection, ECoverInfo> CoverDirectionMap = MakeCoverDirectionMap(CoverDirectionArr);
	ACustomThirdPerson* ControlledPawn = Cast<ACustomThirdPerson>(GetControlledPawn());
	if (!ControlledPawn) { return; }
	UAimingComponent* AimingComp = ControlledPawn->GetAimingComponent();

	BestAimingInfo = AimingComp->GetBestAimingInfo(ActorOnTileLocation, ControlledPawn->AttackRadius, true, CoverDirectionMap);
	float BestProb = BestAimingInfo.Probability;

	float MinimumAimingProb = 0.35;
	float MaximumAimingProb = 0.50;	
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

	CoverDirectionMap.Add(EDirection::East, ECoverInfo::None);
	CoverDirectionMap.Add(EDirection::West, ECoverInfo::None);
	CoverDirectionMap.Add(EDirection::North, ECoverInfo::None);
	CoverDirectionMap.Add(EDirection::South, ECoverInfo::None);

	EDirection Direction = EDirection::None;
	for (FVector CoverDirection : CoverDirectionArr)
	{
		if (CoverDirection.Equals(East)) 
		{ 
			Direction = EDirection::East; 
			UE_LOG(LogTemp, Warning, L"동");

		}
		else if (CoverDirection.Equals(West)) 
		{
			Direction = EDirection::West;
			UE_LOG(LogTemp, Warning, L"서");

		}
		else if (CoverDirection.Equals(North)) 
		{
			Direction = EDirection::North;
			UE_LOG(LogTemp, Warning, L"북");

		}
		else  
		{ 
			Direction = EDirection::South;
			UE_LOG(LogTemp, Warning, L"남");
		}

		CoverDirectionMap.Add(Direction, ECoverInfo::Unknown);

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

void AEnemyController::DebugAimingInfo(const FVector TileLocation, const int32 Score)
{
	FColor DebugPointColor = FColor(0, 0, 0);
	if (Score >= 80)		//빨
	{
		DebugPointColor = FColor(255, 0, 0);
	}
	else if (Score >= 60)	//초
	{
		DebugPointColor = FColor(0, 255, 0);
	}
	else if (Score >= 40)	//파
	{
		DebugPointColor = FColor(0, 0, 255);
	}
	else	//분홍
	{
		DebugPointColor = FColor(255, 0, 255);
	}

	UKismetSystemLibrary::DrawDebugPoint(
		GetWorld(),
		TileLocation + FVector(0, 0, 100),
		20,  					
		DebugPointColor,  
		20
	);
}