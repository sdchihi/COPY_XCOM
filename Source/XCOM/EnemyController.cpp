// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyController.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "TileManager2.h"
#include "CustomThirdPerson.h"
#include "Tile.h"
#include "AimingComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Path.h"
#include "Classes/BehaviorTree/BehaviorTreeComponent.h"
#include "Classes/BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"


void AEnemyController::BeginPlay()
{
	Super::BeginPlay();
	PrimaryActorTick.bCanEverTick = false;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATileManager2::StaticClass(), FoundActors);
	TileManager = Cast<ATileManager2>(FoundActors[0]);

	NextLocationKeyID = BlackboardComp->GetKeyID("NextLocation");
	ActionKeyID = BlackboardComp->GetKeyID("NextAction");
	RemainingMovementKeyID = BlackboardComp->GetKeyID("RemainingMovement");
}


void AEnemyController::SetNextAction()
{
	ACustomThirdPerson* ControlledPawn = Cast<ACustomThirdPerson>(GetPawn());
	if (!ControlledPawn) { return; }
	int32 MovableStep = ControlledPawn->GetMovableStepPerActionPoint();

	ATile* OverllapedTile = TileManager->GetOverlappedTile(ControlledPawn);
	if (!OverllapedTile) { return; }

	TArray<ATile*> MovableTiles;
	TileManager->GetAvailableTiles(OverllapedTile, MovableStep, MovableStep, MovableTiles);

	TMap<ATile*, FAICommandInfo> TileScoreBoard = GetScoreBoard(MovableTiles);
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
		if (bWallAround)	//���� ����.
		{
			for (ACustomThirdPerson* Unit : PlayerCharacters)
			{
				FVector UnitLocation = Unit->GetActorLocation();
				bool bGoodAngle;

				if ( !CheckMimiumInterval(TileLocation, UnitLocation))// �ּ� ���� ���� ���н� Score - 50 �г�Ƽ 
				{
					GeographicalScore -= 50;
				}
				if (IsProtectedByCover(TileLocation, UnitLocation, CoverDirectionArr, bGoodAngle))
				{
					if (bGoodAngle) { GeographicalScore += 6; }
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
			UE_LOG(LogTemp, Warning, L"AI  �̵� ������ Ÿ�� index : %d  ����: (���� ����)%d + (�׼� ����)%d = %d", Index, GeographicalScore, ActionScore, TotalScore);

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

bool AEnemyController::IsProtectedByCover(const FVector TileLocation, const FVector TargetActorLocation, const TArray<FVector> CoverDirectionArr, OUT bool& bGoodAngle) 
{
	FVector DirectionToTargetUnit = (TargetActorLocation - TileLocation).GetSafeNormal2D();
	for (FVector CoverDirection : CoverDirectionArr)
	{
		float AngleBtwTargetAndWall = FMath::RadiansToDegrees(acosf(FVector::DotProduct(DirectionToTargetUnit, CoverDirection)));
		AngleBtwTargetAndWall = FMath::Abs(AngleBtwTargetAndWall);
		if (AngleBtwTargetAndWall < 89)
		{
			if (AngleBtwTargetAndWall < 45) 
			{
				bGoodAngle = true;
			}
			else 
			{
				bGoodAngle = false;
			}
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
	ACustomThirdPerson* ControlledPawn = Cast<ACustomThirdPerson>(GetPawn());
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
			UE_LOG(LogTemp, Warning, L"��");

		}
		else if (CoverDirection.Equals(West)) 
		{
			Direction = EDirection::West;
			UE_LOG(LogTemp, Warning, L"��");

		}
		else if (CoverDirection.Equals(North)) 
		{
			Direction = EDirection::North;
			UE_LOG(LogTemp, Warning, L"��");

		}
		else  
		{ 
			Direction = EDirection::South;
			UE_LOG(LogTemp, Warning, L"��");
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
	for (auto It = TileScoreBoard.CreateConstIterator(); It; ++It)		//�б� ���� Interator    //�б� ����� CreateIterator
	{
		if (HighestScore < It.Value().Score) 
		{
			HighestScore = It.Value().Score;
			HighestScoredTile = It.Key();
		}
	}
	int32 TileIndex = TileManager->ConvertVectorToIndex(HighestScoredTile->GetActorLocation());

	AimingInfo = TileScoreBoard[HighestScoredTile].AimingInfo;
	BlackboardComp->SetValue<UBlackboardKeyType_Bool>(RemainingMovementKeyID, true);
	BlackboardComp->SetValue<UBlackboardKeyType_Enum>(ActionKeyID, static_cast<UBlackboardKeyType_Enum::FDataType>(TileScoreBoard[HighestScoredTile].Action));
	PathToTarget = TileManager->GetPathToTile(TileIndex).OnTheWay;

	UE_LOG(LogTemp, Warning, L"AIŽ�� ��� -  TileIndex  : %d  Scroed : %d", TileIndex, HighestScore);
}

void AEnemyController::RenewNextLocation() 
{
	int32 PathLength = PathToTarget.Num();
	if (MovementIndex < PathLength) 
	{
		FVector NextLoc = TileManager->ConvertIndexToVector(PathToTarget[MovementIndex]);
		BlackboardComp->SetValue<UBlackboardKeyType_Vector>(NextLocationKeyID, NextLoc);
		MovementIndex++;
	}
	else 
	{
		BlackboardComp->SetValue<UBlackboardKeyType_Bool>(RemainingMovementKeyID, false);
		MovementIndex = 0;
	}
}

void AEnemyController::ShootToPlayerUnit() 
{
	//AShooterWeapon* MyWeapon = MyBot ? MyBot->GetWeapon() : NULL;

	ACustomThirdPerson* ControlledPawn = Cast<ACustomThirdPerson>(GetPawn());
	UAimingComponent* AimingComp = ControlledPawn ? ControlledPawn->GetAimingComponent() : nullptr;
	if (AimingComp == nullptr && AimingInfo == nullptr) 
	{
		return;
	}
	ControlledPawn->AttackEnemyAccrodingToState(*AimingInfo);
}


void AEnemyController::DebugAimingInfo(const FVector TileLocation, const int32 Score)
{
	FColor DebugPointColor = FColor(0, 0, 0);
	if (Score >= 80)		//��
	{
		DebugPointColor = FColor(255, 0, 0);
	}
	else if (Score >= 60)	//��
	{
		DebugPointColor = FColor(0, 255, 0);
	}
	else if (Score >= 40)	//��
	{
		DebugPointColor = FColor(0, 0, 255);
	}
	else	//��ȫ
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