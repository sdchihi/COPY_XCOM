// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyController.h"
#include "Classes/Kismet/GameplayStatics.h"
#include "TileManager2.h"
#include "CustomThirdPerson.h"
#include "Tile.h"


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

	
	int32 MaxPoint = 0;
	for (auto SingleMovableTile : MovableTiles) 
	{
		
	}
}


int32 AEnemyController::ScoringOnTile(ATile* AvailableTile)
{
	int32 Score = 0; 

	FVector TileLocation = AvailableTile->GetActorLocation();
	TArray<FVector> CoverDirectionArr;
	bool bWallAround = TileManager->CheckWallAround(TileLocation, CoverDirectionArr);

	if (bWallAround) 
	{
		TArray<ACustomThirdPerson*> PlayerCharacters = GetPlayerCharacters();
		for (ACustomThirdPerson* Unit : PlayerCharacters) 
		{
			FVector UnitLocation = Unit->GetActorLocation();
			float Distance = FVector::Dist2D(UnitLocation, TileLocation);
			if (Distance <= TileManager->GetTileSize() * 5) 
			{
				continue;
			}

			FVector DirectionToTargetUnit = (UnitLocation - TileLocation).GetSafeNormal2D();
			for (FVector CoverDirection : CoverDirectionArr)
			{
				float AngleBtwTargetAndWall = FMath::RadiansToDegrees(acosf(FVector::DotProduct(DirectionToTargetUnit, CoverDirection)));
				AngleBtwTargetAndWall = FMath::Abs(AngleBtwTargetAndWall);
				if (AngleBtwTargetAndWall < 90) 
				{
					//  112233
					
					break;
				}
			}

		}

		
	}

	return Score;
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


