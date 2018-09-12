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
#include "XCOMGameMode.h"
#include "FogOfWarManager.h"
#include "XCOMPlayerController.h"

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

	ACustomThirdPerson* ControlledPawn = Cast<ACustomThirdPerson>(GetPawn());
	ControlledPawn->UnitDeadDelegate.AddUniqueDynamic(this, &AEnemyController::ControlledUnitDead);

	if (PatrolBehavior) 
	{
		SelectedBehavior = PatrolBehavior;
	}
}

void AEnemyController::ControlledUnitDead(ACustomThirdPerson* ControlledUnit)
{
	StopBehaviorTree();
}


void AEnemyController::Possess(APawn* InPawn) 
{
	Super::Possess(InPawn);

	ChangeBehavior(PatrolBehavior);
};


void AEnemyController::SetNextAction()
{
	ACustomThirdPerson* ControlledPawn = Cast<ACustomThirdPerson>(GetPawn());
	if (!ControlledPawn) { return; }
	int32 MovableStep = ControlledPawn->GetMovableStepPerActionPoint();

	ATile* OverllapedTile = TileManager->GetOverlappedTile(ControlledPawn);
	if (!OverllapedTile) 
	{ 
		UE_LOG(LogTemp, Warning, L"타일 문제로 실패")
		return; 
	}

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
		FVector TileLocation = TargeTile->GetActorLocation();
		TArray<FVector> CoverDirectionArr;
		bool bWallAround = TileManager->CheckWallAround(TileLocation, CoverDirectionArr);
		if (bWallAround)	//엄폐 가능.
		{
			ScoreToTile(TargeTile, CoverDirectionArr, TileScoreBoard);
		}
	}

	// 엄폐가능한 블록이 없을때
	if (TileScoreBoard.Num() == 0) 
	{
		for (auto TargeTile : MovableTiles)
		{
			ScoreToTile(TargeTile, TArray<FVector>(), TileScoreBoard);
		}
	}

	//공격 가능한 적이 없을때
	if (TileScoreBoard.Num() == 0)
	{
		AXCOMGameMode* GameMode = Cast<AXCOMGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		FVector PlayerUnitMiddlePoint = GameMode->GetPlayerUnitMiddlePoint();

		float  MinDist = MAX_FLT;
		ATile* NearestTile;
		for (auto TargeTile : MovableTiles)
		{
			float Dist = FVector::Dist2D(TargeTile->GetActorLocation(), PlayerUnitMiddlePoint);
			if (Dist < MinDist) 
			{
				MinDist = Dist;
				NearestTile = TargeTile;
			}
		}
		FAICommandInfo CommandInfo;// = FAICommandInfo(TotalScore, &BestAimingInfo, ActionOnTargetTile);
		CommandInfo.Action = EAction::Vigilance;
		CommandInfo.Score = 1;

		TileScoreBoard.Add(NearestTile, CommandInfo);
	}

	return TileScoreBoard;
}


void AEnemyController::ScoreToTile(ATile* TargetTile, TArray<FVector> CoverDirectionArr , OUT TMap<ATile*, FAICommandInfo>& TileScoreBoard)
{
	FVector TileLocation = TargetTile->GetActorLocation();
	TArray<ACustomThirdPerson*> PlayerCharacters = GetPlayerCharacters();

	int32 GeographicalScore = 0;
	int32 ActionScore = 0;

	for (ACustomThirdPerson* Unit : PlayerCharacters)
	{
		FVector UnitLocation = Unit->GetActorLocation();
		bool bGoodAngle;

		if (!CheckMimiumInterval(TileLocation, UnitLocation))// 최소 간격 유지 실패시 Score - 50 패널티 
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

	EAction ActionOnTargetTile = GetActionAccordingToScore(ActionScore);

	int32 TotalScore = ActionScore + GeographicalScore;

	FAICommandInfo* CommandInfo = new FAICommandInfo();
	CommandInfo->Action = ActionOnTargetTile;
	CommandInfo->AimingInfo = new FAimingInfo(BestAimingInfo);
	CommandInfo->Score = TotalScore;

	TileScoreBoard.Add(TargetTile, *CommandInfo);

	int32 Index = TileManager->ConvertVectorToIndex(TargetTile->GetActorLocation());
	UE_LOG(LogTemp, Warning, L"AI  이동 가능한 타일 index : %d  점수: (지리 점수)%d + (액션 점수)%d = %d", Index, GeographicalScore, ActionScore, TotalScore);
	DebugAimingInfo(TileLocation, TotalScore);
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
	TMap<EDirection, ECoverInfo> CoverDirectionMap = MakeCoverMap(CoverDirectionArr);
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

TMap<EDirection, ECoverInfo> AEnemyController::MakeCoverMap(TArray<FVector> CoverDirectionArr)
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

EAction AEnemyController::GetActionAccordingToScore(int32 ActionScore) const
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
	int32 HighestScore = -50000;
	ATile* HighestScoredTile = nullptr;
	for (auto It = TileScoreBoard.CreateConstIterator(); It; ++It)		//읽기 전용 Interator    //읽기 쓰기는 CreateIterator
	{
		if (HighestScore <= It.Value().Score) 
		{
			HighestScore = It.Value().Score;
			HighestScoredTile = It.Key();
		}
	}

	int32 TileIndex = TileManager->ConvertVectorToIndex(HighestScoredTile->GetActorLocation());

	if (AimingInfo != nullptr)
	{
		delete AimingInfo;
	}
	EAction EnemyAction = TileScoreBoard[HighestScoredTile].Action;
	if (EnemyAction == EAction::Attack) 
	{
		AimingInfo = new FAimingInfo(*TileScoreBoard[HighestScoredTile].AimingInfo);
	}
	DecideWayToProceedBasedLocation(HighestScoredTile->GetActorLocation());
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
	ControlledPawn->InformVisilanceSuccess(ControlledPawn->GetActorLocation(), AimingInfo->TargetLocation);

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
	BehaviorTreeComp->StopTree();
};

void AEnemyController::StartBehaviorTree() 
{
	if (SelectedBehavior)
	{
		BehaviorTreeComp->StartTree(*SelectedBehavior);
	}
	else 
	{
		UE_LOG(LogTemp, Warning, L"Tree 문제 발생");
	}
};

void AEnemyController::StartBehaviorTreeFromDefault()
{
	if (SelectedBehavior)
	{
		BlackboardComp->SetValue<UBlackboardKeyType_Enum>(ActionKeyID, static_cast<UBlackboardKeyType_Enum::FDataType>(EAction::None));
		BehaviorTreeComp->StartTree(*SelectedBehavior);
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

	ATile* TargetTile = nullptr;
	float MinDistance = MAX_FLT;
	for (ATile* SingleTile : MovableTiles)
	{
		float Distance = FVector::Dist2D(SingleTile->GetActorLocation(), PatrolTargetLocation);
		if (Distance < MinDistance)
		{
			MinDistance = Distance;
			TargetTile = SingleTile;
		}
	}

	if (TargetTile == nullptr) 
	{
		UE_LOG(LogTemp, Warning, L"정찰 불가한 상태에 빠진 액터 발생! ");
		return;
	}
	FVector TileLocation = TargetTile->GetActorLocation();
	int32 TileIndex = TileManager->ConvertVectorToIndex(TileLocation);

	DecideWayToProceedBasedLocation(TileLocation);
	TArray<FVector> Tempor;
	for (int32 PathIndex : TileManager->GetPathToTile(TileIndex).OnTheWay)
	{
		FVector PathLocation = TileManager->ConvertIndexToVector(PathIndex);
		PathLocation += FVector(-50, -50, 0);
		Tempor.Add(PathLocation);
	}
	PathToTarget = Tempor;
};

void AEnemyController::ChangeBehaviorToCombat() 
{
	if (CombatBehavior) 
	{
		ChangeBehavior(CombatBehavior);
		UE_LOG(LogTemp, Warning, L"컴뱃으로 변경! ");
	}
}

void AEnemyController::ChangeBehavior(UBehaviorTree* BehaviorTreeToSet)
{
	if (BehaviorTreeToSet)
	{
		SelectedBehavior = BehaviorTreeToSet;
		if (SelectedBehavior->BlackboardAsset)
		{
			BlackboardComp->InitializeBlackboard(*BehaviorTreeToSet->BlackboardAsset);
		}

		NextLocationKeyID = BlackboardComp->GetKeyID("NextLocation");
		ActionKeyID = BlackboardComp->GetKeyID("NextAction");
		RemainingMovementKeyID = BlackboardComp->GetKeyID("RemainingMovement");
		FormalProceedKeyID = BlackboardComp->GetKeyID("FormalProceed");
	}
}

bool AEnemyController::CheckTargetLocation(FVector TargetLocation) 
{
	AXCOMGameMode* GameMode = Cast<AXCOMGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	bool IsInCog = GameMode->GetFowManager()->IsInCognitionArea(TargetLocation);
	if (IsInCog) 
	{
		UE_LOG(LogTemp, Warning, L"추적 가능");
	}
	else
	{
		UE_LOG(LogTemp, Warning, L"추적 불가");
	}
	return IsInCog;
}

void AEnemyController::OrderStartVigilance()
{
	AXCOMGameMode* GameMode = Cast<AXCOMGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	ACustomThirdPerson* ControlledPawn = Cast<ACustomThirdPerson>(GetPawn());

	bool PlayersUnitTeamFlag = !ControlledPawn->GetTeamFlag();

	ControlledPawn->BindVigilanceEvent(GameMode->GetTeamMemeber(PlayersUnitTeamFlag));
	ControlledPawn->bInVisilance = true;
	ControlledPawn->AnnounceVisilance();
	ControlledPawn->UseActionPoint(2);
}

void AEnemyController::DecideWayToProceedBasedLocation(FVector TargetLocation) 
{
	bool bIsInCog = CheckTargetLocation(TargetLocation);

	if (bIsInCog) 
	{
		AXCOMPlayerController* PlayerController = Cast<AXCOMPlayerController>(GetWorld()->GetFirstPlayerController());
		ACustomThirdPerson* ControlledPawn = Cast<ACustomThirdPerson>(GetPawn());

		PlayerController->EnableFocusing(ControlledPawn, false);
		BlackboardComp->SetValue<UBlackboardKeyType_Bool>(FormalProceedKeyID, true);
		UE_LOG(LogTemp, Warning, L"Formal ");
	}
	else 
	{
		BlackboardComp->SetValue<UBlackboardKeyType_Bool>(FormalProceedKeyID, false);
		UE_LOG(LogTemp, Warning, L"Informal");
	}
}