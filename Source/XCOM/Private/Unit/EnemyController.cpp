// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyController.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "TileManager.h"
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
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"


AEnemyController::AEnemyController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	BlackboardComp = ObjectInitializer.CreateDefaultSubobject<UBlackboardComponent>(this, TEXT("BlackBoardComp"));
	BrainComponent = BehaviorTreeComp = ObjectInitializer.CreateDefaultSubobject<UBehaviorTreeComponent>(this, TEXT("BehaviorComp"));
	bWantsPlayerState = true;
}


void AEnemyController::BeginPlay()
{
	Super::BeginPlay();
	PrimaryActorTick.bCanEverTick = false;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATileManager::StaticClass(), FoundActors);
	TileManager = Cast<ATileManager>(FoundActors[0]);

}

void AEnemyController::Possess(APawn* InPawn) 
{
	Super::Possess(InPawn);

	if (EnemyBehavior)
	{
		if (EnemyBehavior->BlackboardAsset)
		{
			BlackboardComp->InitializeBlackboard(*EnemyBehavior->BlackboardAsset);
		}

		NextLocationKeyID = BlackboardComp->GetKeyID("NextLocation");
		ActionKeyID = BlackboardComp->GetKeyID("NextAction");
		RemainingMovementKeyID = BlackboardComp->GetKeyID("RemainingMovement");

		/*BlackboardComp->SetValue<UBlackboardKeyType_Enum>(ActionKeyID, static_cast<UBlackboardKeyType_Enum::FDataType>(EAction::None));

		BehaviorTreeComp->StartTree(*EnemyBehavior);*/
	}
};


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
		if (bWallAround)	//엄폐 가능.
		{
			for (ACustomThirdPerson* Unit : PlayerCharacters)
			{
				FVector UnitLocation = Unit->GetActorLocation();
				bool bGoodAngle;

				if ( !CheckMimiumInterval(TileLocation, UnitLocation))// 최소 간격 유지 실패시 Score - 50 패널티 
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
			CommandInfo.AimingInfo = new FAimingInfo(BestAimingInfo);
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

void AEnemyController::FindBestScoredAction(const TMap<ATile*, FAICommandInfo>& TileScoreBoard)
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

	if (AimingInfo) 
	{
		delete AimingInfo;
	}

	AimingInfo = new FAimingInfo(*TileScoreBoard[HighestScoredTile].AimingInfo);
	BlackboardComp->SetValue<UBlackboardKeyType_Bool>(RemainingMovementKeyID, true);
	BlackboardComp->SetValue<UBlackboardKeyType_Enum>(ActionKeyID, static_cast<UBlackboardKeyType_Enum::FDataType>(TileScoreBoard[HighestScoredTile].Action));
	

	TArray<FVector> Tempor;
	for (int32 PathIndex : TileManager->GetPathToTile(TileIndex).OnTheWay)
	{
		FVector PathLocation = TileManager->ConvertIndexToVector(PathIndex);
		PathLocation += FVector(-50, -50, 0);
		Tempor.Add(PathLocation);
	}
	PathToTarget = Tempor;

	for (auto It = TileScoreBoard.CreateConstIterator(); It; ++It)		//읽기 전용 Interator    //읽기 쓰기는 CreateIterator
	{
		if (HighestScore < It.Value().Score)
		{
			delete It.Value().AimingInfo;
		}
	}

	UE_LOG(LogTemp, Warning, L"AI탐색 결과 -  TileIndex  : %d  Scroed : %d", TileIndex, HighestScore);
}

void AEnemyController::FollowThePath() 
{
	ACustomThirdPerson* ControlledPawn = Cast<ACustomThirdPerson>(GetPawn());
	if (!ControlledPawn) { return; }
	ControlledPawn->MoveToTargetTile(&PathToTarget, 1);
}



void AEnemyController::ShootToPlayerUnit() 
{
	ACustomThirdPerson* ControlledPawn = Cast<ACustomThirdPerson>(GetPawn());
	UAimingComponent* AimingComp = ControlledPawn ? ControlledPawn->GetAimingComponent() : nullptr;
	if (AimingComp == nullptr || AimingInfo == nullptr) 
	{
		return;
	}
	ControlledPawn->AttackEnemyAccrodingToState(*AimingInfo);
	UE_LOG(LogTemp, Warning, L"%s 에서 적이 사격시작", *AimingInfo->StartLocation.ToString())
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


void AEnemyController::StopBehaviorTree() 
{
	UE_LOG(LogTemp, Warning, L"스톱 ~");
	BehaviorTreeComp->StopTree();
};

void AEnemyController::StartBehaviorTree() 
{
	if (EnemyBehavior)
	{
		BehaviorTreeComp->StartTree(*EnemyBehavior);
	}
};

void AEnemyController::StartBehaviorTreeFromDefault()
{
	
	if (EnemyBehavior)
	{
		BlackboardComp->SetValue<UBlackboardKeyType_Enum>(ActionKeyID, static_cast<UBlackboardKeyType_Enum::FDataType>(EAction::None));
		BehaviorTreeComp->StartTree(*EnemyBehavior);
		UE_LOG(LogTemp, Warning, L"시작! ");
	}
};



void AEnemyController::SetNextPatrolLocation() 
{
	ACustomThirdPerson* ControlledPawn = Cast<ACustomThirdPerson>(GetPawn());
	if (!ControlledPawn) { return; }
	int32 MovableStep = ControlledPawn->GetMovableStepPerActionPoint();

	ATile* OverllapedTile = TileManager->GetOverlappedTile(ControlledPawn);
	if (!OverllapedTile) { return; }

	TArray<ATile*> MovableTiles;
	TileManager->GetAvailableTiles(OverllapedTile, MovableStep, MovableStep, MovableTiles);

	FVector East = FVector(-1, 0, 0);
	FVector West = FVector(1, 0, 0);
	FVector North = FVector(0, 1, 0);
	FVector South = FVector(0, -1, 0);

	ATile* TargetTile;
	float MaxDistance = 0;
	for (ATile* SingleTile : MovableTiles)
	{
		FVector DirectionToTile = (SingleTile->GetActorLocation() - ControlledPawn->GetActorLocation());
		if (CheckHavingDirectionComponent(DirectionToTile)) 
		{
			float Distance = FVector::Dist2D(SingleTile->GetActorLocation(), ControlledPawn->GetActorLocation());
			if (MaxDistance < Distance) 
			{
				MaxDistance = Distance; 
				TargetTile = SingleTile;
			}
		}
	}

	if (TargetTile == nullptr) 
	{
		UE_LOG(LogTemp, Warning, L"정찰 불가한 상태에 빠진 액터 발생! ");
		return;
	}

	int32 TileIndex = TileManager->ConvertVectorToIndex(TargetTile->GetActorLocation());
	TArray<FVector> Tempor;
	for (int32 PathIndex : TileManager->GetPathToTile(TileIndex).OnTheWay)
	{
		FVector PathLocation = TileManager->ConvertIndexToVector(PathIndex);
		PathLocation += FVector(-50, -50, 0);
		Tempor.Add(PathLocation);
	}
	PathToTarget = Tempor;

	/*
	if (EnemyBehavior)
	{
		BlackboardComp->SetValue<UBlackboardKeyType_Enum>(ActionKeyID, static_cast<UBlackboardKeyType_Enum::FDataType>(EAction::None));
		BehaviorTreeComp->StartTree(*EnemyBehavior);
		UE_LOG(LogTemp, Warning, L"시작! ");
	}
	*/
};

bool AEnemyController::CheckHavingDirectionComponent(FVector VectorToCheck) 
{
	switch (PatrolDirection) 
	{
	case EDirection::East:
		if (FMath::IsNegativeFloat(VectorToCheck.X)) 
		{
			return true;
		}
		break;
	case EDirection::West:
		if (0 < VectorToCheck.X)
		{
			return true;
		}
		break;
	case EDirection::North:
		if (0 < VectorToCheck.Y)
		{
			return true;
		}
		break;
	case EDirection::South:
		if (FMath::IsNegativeFloat(VectorToCheck.Y))
		{
			return true;
		}
		break;
	default:
		break;
	}

	return false;
}
