
#include "AimingComponent.h"
#include "CustomThirdPerson.h"
#include "Classes/Kismet/KismetSystemLibrary.h"
#include "DestructibleWall.h"
#include "Gun.h"

UAimingComponent::UAimingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UAimingComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAimingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UAimingComponent::GetAttackableEnemyInfo(const FVector ActorLocation , FUnitStatus const * const Status, const bool bIsCover, const TMap<EDirection, ECoverInfo>& CoverDirectionMap, OUT TArray<FAimingInfo>& AimingInfoList)
{
	TArray<ACustomThirdPerson*> EnemyInRange;
	if (GetEnemyInRange(ActorLocation, Status->AttackRadius, EnemyInRange) == false)
	{
		return;
	};

	TArray<FHitResult> AttackableEnemysHitResult;
	if (FilterAttackableEnemy(ActorLocation, CoverDirectionMap , EnemyInRange, bIsCover, AttackableEnemysHitResult) == false)
	{
		return;
	};

	TArray<FAimingInfo> AimingInfoInAllCase;
	TArray<FVector> TargetLocationArr;
	for (FHitResult FilteredHitResult : AttackableEnemysHitResult)
	{
		APawn* DetectedPawn = Cast<APawn>(FilteredHitResult.GetActor());
		if (DetectedPawn)
		{
			TMap<EAimingFactor, float> AimingFactor;
			TMap<ECriticalFactor, int8> CriticalFactor;

			float Probability = CalculateAttackSuccessRatio(ActorLocation, FilteredHitResult, Status, bIsCover, DetectedPawn, AimingFactor, CriticalFactor);
			FVector StartLocation = FilteredHitResult.TraceStart;
			FVector TargetLocation = DetectedPawn->GetActorLocation();
			TargetLocationArr.AddUnique(TargetLocation);
			AimingInfoInAllCase.Add(FAimingInfo(StartLocation, TargetLocation, Probability, DetectedPawn, AimingFactor, CriticalFactor));
		}
	}

	FindBestCaseInAimingInfo(AimingInfoInAllCase, AimingInfoList, TargetLocationArr);
};

float UAimingComponent::CalculateAttackSuccessRatio(const FVector ActorLocation, const FHitResult HitResult, FUnitStatus const * const Status, const bool bIsCover, APawn* TargetPawn, TMap<EAimingFactor, float>& AimingFactor, TMap<ECriticalFactor, int8>& CriticalFactor)
{
	AimingFactor.Add(EAimingFactor::AimingAbility, Status->Aiming);
	float FailureRatio = 0;
	FVector AimDirection = (ActorLocation - HitResult.GetActor()->GetActorLocation()).GetSafeNormal();
	ACustomThirdPerson* TargetThirdPerson = Cast<ACustomThirdPerson>(TargetPawn);

	if (!TargetThirdPerson){ return 0; }

	if (TargetThirdPerson->bIsCovering)
	{
		float AngleBetweenAimAndWall = 0;
		ECoverInfo CoverInfo = ECoverInfo::None;
		AngleBetweenAimAndWall = CalculateAngleBtwAimAndWall(AimDirection, TargetThirdPerson, CoverInfo);

		if (AngleBetweenAimAndWall < 90)
		{
			// 장애물에 의한 실패 확률 상승
			float FailureDueToCover = 0;
			if (CoverInfo == ECoverInfo::HalfCover)
			{
				FailureDueToCover = 0.2;
				AimingFactor.Add(EAimingFactor::HalfCover, -FailureDueToCover);
			}
			else if (CoverInfo == ECoverInfo::FullCover)
			{
				FailureDueToCover = 0.4;
				AimingFactor.Add(EAimingFactor::FullCover, -FailureDueToCover);
			}
			FailureRatio += FailureDueToCover;
		}
	}

	float AngleBtwActor = FMath::RadiansToDegrees(acosf(FVector::DotProduct(AimDirection, TargetThirdPerson->GetActorForwardVector())));
	if (45 < AngleBtwActor)
	{
		int8 CriticalBonus = 20;
		if (70 < AngleBtwActor) 
		{
			CriticalBonus = 40;
		}
		CriticalFactor.Add(ECriticalFactor::SideAttack, CriticalBonus);
	}

	// Todo - 적과의 거리에 따른 실패 확률 상승 ( 수정 필요 )
	float FailureDueToDistance = (HitResult.Distance * (15 / Status->AttackRadius)) / 100;
	FailureRatio += FailureDueToDistance;
	AimingFactor.Add(EAimingFactor::Disatnce, -FailureDueToDistance);
	
	UE_LOG(LogTemp, Warning, L"최종 공격 성공 확률 : %f", 0.8f - FailureRatio);

	return Status->Aiming - FailureRatio;
}

bool UAimingComponent::GetEnemyInRange(const FVector ActorLocation, const float AttackRadius, OUT TArray<ACustomThirdPerson*>& CharacterInRange)
{
	TArray<TEnumAsByte<EObjectTypeQuery>> UnUsedObjectType;
	TArray<AActor*> ActorsToIgnore;
	TArray<AActor*> ActorsInRange;

	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), ActorLocation, AttackRadius, UnUsedObjectType, ACustomThirdPerson::StaticClass(), ActorsToIgnore, ActorsInRange);
	if (ActorsInRange.Num() == 0) { return false; }

	for (AActor* SingleActor : ActorsInRange)
	{
		ACustomThirdPerson* EnemyCharacter = Cast<ACustomThirdPerson>(SingleActor);
		if (EnemyCharacter)
		{
			CharacterInRange.Add(EnemyCharacter);
		}
	}
	if (CharacterInRange.Num() == 0) { return false; }

	return true;
}

float UAimingComponent::CalculateAngleBtwAimAndWall(const FVector AimDirection, ACustomThirdPerson* TargetPawn, OUT ECoverInfo& CoverInfo)
{
	float MinAngleBetweenAimAndWall = MAX_FLT;
	FVector North = FVector(0, 1, 0);
	FVector South = FVector(0, -1, 0);
	FVector West = FVector(-1, 0, 0);
	FVector East = FVector(1, 0, 0);

	for (auto CoverDirectionState : TargetPawn->CoverDirectionMap)
	{
		FVector WallForwardVector;
		float AngleBetweenAimAndWall = 0;
		if (CoverDirectionState.Value != ECoverInfo::None)
		{
			switch (CoverDirectionState.Key)
			{
			case EDirection::East:
				WallForwardVector = East;
				break;
			case EDirection::West:
				WallForwardVector = West;
				break;
			case EDirection::North:
				WallForwardVector = North;
				break;
			case EDirection::South:
				WallForwardVector = South;
				break;
			default:
				break;
			}
			AngleBetweenAimAndWall = FMath::RadiansToDegrees(acosf(FVector::DotProduct(AimDirection, WallForwardVector)));
			if (MinAngleBetweenAimAndWall > AngleBetweenAimAndWall)
			{
				CoverInfo = CoverDirectionState.Value;
				MinAngleBetweenAimAndWall = AngleBetweenAimAndWall;
			}
		}
	}
	return MinAngleBetweenAimAndWall;
}

bool UAimingComponent::FilterAttackableEnemy(const FVector ActorLocation, const TMap<EDirection, ECoverInfo>& CoverDirectionMap, const TArray<ACustomThirdPerson*>& EnemiesInRange, const bool bIsCovering, OUT TArray<FHitResult>& SensibleEnemyInfo)
{
	TArray<FHitResult> ResultRequireInspection;

	//엄폐 상태일때 다각도에서 확인이 필요함
	if (bIsCovering)
	{
		for (auto CoverDirection : CoverDirectionMap)
		{
			if (CoverDirection.Value != ECoverInfo::None)
			{
				FRotator Direction = FindCoverDirection(CoverDirection);

				FVector RightVector = FVector::CrossProduct(FVector::UpVector, Direction.Vector());
				FVector RightSide = ActorLocation + RightVector * 100;
				FVector LeftSide = ActorLocation - RightVector * 100;

				TArray<FHitResult> SurroundingAreaInfo;
				GetAimingInfoFromSurroundingArea(ActorLocation, RightSide, SurroundingAreaInfo);
				GetAimingInfoFromSurroundingArea(ActorLocation, LeftSide, SurroundingAreaInfo);

				for (FHitResult SingleSurroundingAreaInfo : SurroundingAreaInfo)
				{
					for (ACustomThirdPerson* SingleTargetEnemy : EnemiesInRange)
					{
						FHitResult HitResult = LineTraceWhenAiming(SingleSurroundingAreaInfo.TraceEnd, SingleTargetEnemy->GetActorLocation());
						ResultRequireInspection.Add(HitResult);
					}
				}

			}
		}
	}
	// 엄폐아닌 위치에서 확인
	for (ACustomThirdPerson* SingleTargetEnemy : EnemiesInRange)
	{
		FHitResult HitResult = LineTraceWhenAiming(ActorLocation, SingleTargetEnemy->GetActorLocation());
		ResultRequireInspection.Add(HitResult);
	}

	for (FHitResult SingleHitResult : ResultRequireInspection)
	{
		ACustomThirdPerson* DetectedPawn = Cast<ACustomThirdPerson>(SingleHitResult.GetActor());
		if (DetectedPawn)
		{
			SensibleEnemyInfo.Add(SingleHitResult);
		}
	}
	if (SensibleEnemyInfo.Num() == 0)
	{
		return false;
	}

	return true;
}

FHitResult UAimingComponent::LineTraceWhenAiming(const FVector StartLocation, const FVector TargetLocation)
{
	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams CollisionParams;
	CollisionParams.TraceTag = TraceTag;
	CollisionParams.bFindInitialOverlaps = false;
	
	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		TargetLocation,
		GetAimingChannel(),
		CollisionParams
	);
	return HitResult;
}

void UAimingComponent::GetAimingInfoFromSurroundingArea(const FVector ActorLocation, const FVector SurroundingArea, TArray<FHitResult>&  AimingInfo)
{
	FHitResult HitResultFromCharToSurroundingArea = LineTraceWhenAiming(ActorLocation, SurroundingArea);
	if (!HitResultFromCharToSurroundingArea.GetActor())
	{
		AimingInfo.Add(HitResultFromCharToSurroundingArea);
	}
}

FRotator UAimingComponent::FindCoverDirection(TPair<EDirection, ECoverInfo> DirectionAndInfoPair)
{
	FRotator Direction = FRotator(0, 0, 0);
	switch (DirectionAndInfoPair.Key)
	{
	case EDirection::East:
		Direction = FRotator(0, 0, 0);
		break;
	case EDirection::West:
		Direction = FRotator(0, 180, 0);
		break;
	case EDirection::North:
		Direction = FRotator(0, 90, 0);
		break;
	case EDirection::South:
		Direction = FRotator(0, 270, 0);
		break;
	}
	return Direction;
}

void UAimingComponent::FindBestCaseInAimingInfo(const TArray<FAimingInfo> AllCaseInfo, TArray<FAimingInfo>& BestCaseArr, const TArray<FVector> TargetLocArr)
{	
	ACustomThirdPerson* OwnerActor = Cast<ACustomThirdPerson>(GetOwner());
	int8 CriticalValue = OwnerActor->GetGunReference().GetCriticalAbil();

	for (FVector TargetLoc : TargetLocArr)
	{
		FAimingInfo SingleBestCase;
		float HighestProbability = 0;
		for (FAimingInfo SingleCaseInCheck : AllCaseInfo)
		{
			if (SingleCaseInCheck.TargetLocation.Equals(TargetLoc)) 
			{
				if (HighestProbability < SingleCaseInCheck.Probability)
				{
					HighestProbability = SingleCaseInCheck.Probability;
					SingleBestCase = SingleCaseInCheck;
				}
			}
		}
		if (CriticalValue != 0) 
		{
			SingleBestCase.CriticalFactor.Add(ECriticalFactor::WeaponAbility, CriticalValue);
		}
		BestCaseArr.Add(SingleBestCase);
	}
}

bool UAimingComponent::GetVigilanceAimingInfo(FUnitStatus const * const Status, const bool bIsCover, const TMap<EDirection, ECoverInfo>& CoverDirectionMap, const FVector TargetLocation, OUT FAimingInfo& AimingInfo)
{
	TArray<FHitResult> UnprotectdEnemyHitResultArray;
	FVector ActorLocation = GetOwner()->GetActorLocation();

	TArray<FHitResult> ResultRequireInspection;
	if (bIsCover)
	{
		for (auto CoverDirection : CoverDirectionMap)
		{
			if (CoverDirection.Value != ECoverInfo::None)
			{
				FRotator Direction = FindCoverDirection(CoverDirection);
				FVector RightVector = FVector::CrossProduct(FVector::UpVector, Direction.Vector());
				FVector RightSide = GetOwner()->GetActorLocation() + RightVector * 100;
				FVector LeftSide = GetOwner()->GetActorLocation() - RightVector * 100;

				TArray<FHitResult> SurroundingAreaInfo;
				GetAimingInfoFromSurroundingArea(ActorLocation, RightSide, SurroundingAreaInfo);
				GetAimingInfoFromSurroundingArea(ActorLocation, LeftSide, SurroundingAreaInfo);

				for (FHitResult SingleSurroundingAreaInfo : SurroundingAreaInfo)
				{
					FHitResult HitResult = LineTraceWhenAiming(SingleSurroundingAreaInfo.TraceEnd, TargetLocation);
					ResultRequireInspection.Add(HitResult);
				}

			}
		}
	}
	FHitResult HitResult = LineTraceWhenAiming(GetOwner()->GetActorLocation(), TargetLocation);
	ResultRequireInspection.Add(HitResult);
	for (FHitResult SingleHitResult : ResultRequireInspection)
	{
		ACustomThirdPerson* DetectedPawn = Cast<ACustomThirdPerson>(SingleHitResult.GetActor());
		if (DetectedPawn)
		{
			UnprotectdEnemyHitResultArray.Add(SingleHitResult);
		}
	}
	if (UnprotectdEnemyHitResultArray.Num() == 0)
	{
		return false;
	}

	TArray<FAimingInfo> AimingInfoInAllCase;
	TArray<FVector> TargetLocationArr;
	TArray<FAimingInfo> BestCaseArr;
	for (FHitResult FilteredHitResult : UnprotectdEnemyHitResultArray)
	{
		APawn* DetectedPawn = Cast<APawn>(FilteredHitResult.GetActor());
		if (DetectedPawn)
		{
			TMap<EAimingFactor, float> AimingFactor;
			TMap<ECriticalFactor, int8> CriticalFactor;

			float Probability = CalculateAttackSuccessRatio(ActorLocation, FilteredHitResult, Status, bIsCover,DetectedPawn, AimingFactor, CriticalFactor);
			FVector StartLocation = FilteredHitResult.TraceStart;
			FVector TargetLocation = DetectedPawn->GetActorLocation();
			TargetLocationArr.AddUnique(TargetLocation);
			AimingInfoInAllCase.Add(FAimingInfo(StartLocation, TargetLocation, Probability, DetectedPawn, AimingFactor, CriticalFactor));
		}
	}
	FindBestCaseInAimingInfo(AimingInfoInAllCase, BestCaseArr, TargetLocationArr);
	AimingInfo = BestCaseArr[0];
	
	return true;
};

FAimingInfo UAimingComponent::GetBestAimingInfo(const FVector ActorLocation, FUnitStatus const * const Status, const bool bIsCover, const TMap<EDirection, ECoverInfo>& CoverDirectionMap)
{
	TArray<FAimingInfo> AimingInfoList;

	GetAttackableEnemyInfo(ActorLocation, Status, bIsCover, CoverDirectionMap, AimingInfoList);

	float HighestProbability = 0;
	FAimingInfo BestAimingInfo;
	for (FAimingInfo AimingInfo : AimingInfoList) 
	{
		if (HighestProbability < AimingInfo.Probability)
		{
			HighestProbability = AimingInfo.Probability;
			BestAimingInfo = AimingInfo;
		}
	}
	return BestAimingInfo;
};

/**
* 현재 Actor가 조준에 사용할 TraceChannel을 가져옵니다.
* @return ECollisionChannel 조준 채널
*/
ECollisionChannel UAimingComponent::GetAimingChannel() 
{
	ACustomThirdPerson* OwnCharacter = Cast<ACustomThirdPerson>(GetOwner());
	if (OwnCharacter->GetTeamFlag()) 
	{
		return ECollisionChannel::ECC_GameTraceChannel8;
	}
	else 
	{
		return ECollisionChannel::ECC_GameTraceChannel10;
	}
}
