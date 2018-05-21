
#include "AimingComponent.h"
#include "CustomThirdPerson.h"
#include "Classes/Kismet/KismetSystemLibrary.h"
#include "DestructibleWall.h"

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

void UAimingComponent::GetAttackableEnemyInfo(const FVector ActorLocation ,const float AttackRadius, const bool bIsCover, const TMap<EDirection, ECoverInfo>& CoverDirectionMap, OUT TArray<FAimingInfo>& AimingInfoList)
{
	TArray<ACustomThirdPerson*> EnemyInRange;
	if (GetEnemyInRange(ActorLocation, AttackRadius, EnemyInRange) == false)
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
			float Probability = CalculateAttackSuccessRatio(ActorLocation, FilteredHitResult, AttackRadius, bIsCover, DetectedPawn, AimingFactor);
			FVector StartLocation = FilteredHitResult.TraceStart;
			FVector TargetLocation = DetectedPawn->GetActorLocation();
			TargetLocationArr.AddUnique(TargetLocation);
			AimingInfoInAllCase.Add(FAimingInfo(StartLocation, TargetLocation, Probability, AimingFactor));
		}
	}

	FindBestCaseInAimingInfo(AimingInfoInAllCase, AimingInfoList, TargetLocationArr);
	UE_LOG(LogTemp, Warning, L"감지 결과 검출된 Target 수 : %d ", AimingInfoList.Num());
};


/**
* 공격 성공 확률을 계산합니다.
* @param HitResult - 공격 대상을 향해 RayCast 결과 얻어진 결과입니다.
* @param TargetPawn - 공격 대상이 되는 Pawn입니다.
* @return 공격 성공 확률을 반환합니다.
*/
float UAimingComponent::CalculateAttackSuccessRatio(const FVector ActorLocation, const FHitResult HitResult, float AttackRadius, const bool bIsCover, APawn* TargetPawn, TMap<EAimingFactor, float>& AimingFactor)
{
	AimingFactor.Add(EAimingFactor::AimingAbility, 0.8);
	float FailureRatio = 0;
	FVector AimDirection = (ActorLocation - HitResult.GetActor()->GetActorLocation()).GetSafeNormal();
	ACustomThirdPerson* TargetThirdPerson = Cast<ACustomThirdPerson>(TargetPawn);
	// 클래스가 유연하지 않으므로 이후 수정
	if (!TargetThirdPerson)
	{
		UE_LOG(LogTemp, Warning, L"벽에 의해 가로막힘");
		return 0;
	}

	//엄폐 상태 확인 수정
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
			UE_LOG(LogTemp, Warning, L"엄폐로 인한 실패 확률 계산 결과 : %f", FailureDueToCover);
		}
	}
	// Todo - 적과의 거리에 따른 실패 확률 상승 ( 수정 필요 )
	float FailureDueToDistance = (HitResult.Distance * (15 / AttackRadius)) / 100;
	FailureRatio += FailureDueToDistance;
	AimingFactor.Add(EAimingFactor::Disatnce, -FailureDueToDistance);

	
	UE_LOG(LogTemp, Warning, L"최종 공격 성공 확률 : %f", 0.8f - FailureRatio);

	return 0.8f - FailureRatio;
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

/**
* 공격 대상이 엄폐하고있는 벽과 조준선이 이루는 각도를 계산합니다.
* @param AimDirection - 조준선의 방향 벡터입니다.
* @param TargetPawn - 공격 대상이 되는 Pawn입니다.
* @return 벽과 조준선이 이루는 각도를 Degree로 반환합니다.
*/
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
				//RelativeCoverLoc = WallForwardVector;
				MinAngleBetweenAimAndWall = AngleBetweenAimAndWall;
			}
		}
	}
	UE_LOG(LogTemp, Warning, L"Minimum Angle : %d ", MinAngleBetweenAimAndWall);

	//RelativeCoverLoc *= 100;

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
		BestCaseArr.Add(SingleBestCase);
	}
}



bool UAimingComponent::GetVigilanceAimingInfo(const float AttackRadius, const bool bIsCover, const TMap<EDirection, ECoverInfo>& CoverDirectionMap, const FVector TargetLocation, OUT FAimingInfo& AimingInfo)
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
	//필터링 끝


	TArray<FAimingInfo> AimingInfoInAllCase;
	TArray<FVector> TargetLocationArr;
	TArray<FAimingInfo> BestCaseArr;
	for (FHitResult FilteredHitResult : UnprotectdEnemyHitResultArray)
	{
		APawn* DetectedPawn = Cast<APawn>(FilteredHitResult.GetActor());
		if (DetectedPawn)
		{
			TMap<EAimingFactor, float> AimingFactor;
			float Probability = CalculateAttackSuccessRatio(ActorLocation, FilteredHitResult, AttackRadius, bIsCover,DetectedPawn, AimingFactor);
			FVector StartLocation = FilteredHitResult.TraceStart;
			FVector TargetLocation = DetectedPawn->GetActorLocation();
			TargetLocationArr.AddUnique(TargetLocation);
			AimingInfoInAllCase.Add(FAimingInfo(StartLocation, TargetLocation, Probability, AimingFactor));

			//수정필요
			DetectedPawn->CustomTimeDilation = 0.05;
		}
	}
	FindBestCaseInAimingInfo(AimingInfoInAllCase, BestCaseArr, TargetLocationArr);
	
	AimingInfo = BestCaseArr[0];
	
	return true;
};



FAimingInfo UAimingComponent::GetBestAimingInfo(const FVector ActorLocation, const float AttackRadius, const bool bIsCover, const TMap<EDirection, ECoverInfo>& CoverDirectionMap)
{
	TArray<FAimingInfo> AimingInfoList;

	GetAttackableEnemyInfo(ActorLocation, AttackRadius, bIsCover, CoverDirectionMap, AimingInfoList);

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
