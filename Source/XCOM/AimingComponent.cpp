
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

void UAimingComponent::GetAttackableEnemyInfo(const float AttackRadius, const bool bIsCover, const TMap<ECoverDirection, ECoverInfo>& CoverDirectionMap, OUT TArray<FAimingInfo>& AimingInfoList)
{
	TArray<ACustomThirdPerson*> EnemyInRange;
	if (GetEnemyInRange(AttackRadius,EnemyInRange) == false)
	{
		return;
	};

	TArray<FHitResult> AttackableEnemysHitResult;
	if (FilterAttackableEnemy(CoverDirectionMap,EnemyInRange, bIsCover, AttackableEnemysHitResult) == false)
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
			float Probability = CalculateAttackSuccessRatio(FilteredHitResult, AttackRadius, DetectedPawn, AimingFactor);
			FVector StartLocation = FilteredHitResult.TraceStart;
			FVector TargetLocation = DetectedPawn->GetActorLocation();
			TargetLocationArr.AddUnique(TargetLocation);
			AimingInfoInAllCase.Add(FAimingInfo(StartLocation, TargetLocation, Probability, AimingFactor));
		}
	}

	FindBestCaseInAimingInfo(AimingInfoInAllCase, AimingInfoList, TargetLocationArr);
	UE_LOG(LogTemp, Warning, L"���� ��� ����� Target �� : %d ", AimingInfoList.Num());
};


/**
* ���� ���� Ȯ���� ����մϴ�.
* @param HitResult - ���� ����� ���� RayCast ��� ����� ����Դϴ�.
* @param TargetPawn - ���� ����� �Ǵ� Pawn�Դϴ�.
* @return ���� ���� Ȯ���� ��ȯ�մϴ�.
*/
float UAimingComponent::CalculateAttackSuccessRatio(const FHitResult HitResult, float AttackRadius, APawn* TargetPawn, TMap<EAimingFactor, float>& AimingFactor)
{
	AimingFactor.Add(EAimingFactor::AimingAbility, 0.8);
	float FailureRatio = 0;
	FVector AimDirection = (GetOwner()->GetActorLocation() - HitResult.GetActor()->GetActorLocation()).GetSafeNormal();
	ACustomThirdPerson* TargetThirdPerson = Cast<ACustomThirdPerson>(TargetPawn);
	// Ŭ������ �������� �����Ƿ� ���� ����
	if (!TargetThirdPerson)
	{
		UE_LOG(LogTemp, Warning, L"���� ���� ���θ���");
		return 0;
	}

	//���� ���� Ȯ�� ����
	if (TargetThirdPerson->bIsCovering)
	{
		float AngleBetweenAimAndWall = 0;
		ECoverInfo CoverInfo = ECoverInfo::None;
		AngleBetweenAimAndWall = CalculateAngleBtwAimAndWall(AimDirection, TargetThirdPerson, CoverInfo);

		if (AngleBetweenAimAndWall < 90)
		{
			// ��ֹ��� ���� ���� Ȯ�� ���
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
			UE_LOG(LogTemp, Warning, L"����� ���� ���� Ȯ�� ��� ��� : %f", FailureDueToCover);
		}
	}
	// Todo - ������ �Ÿ��� ���� ���� Ȯ�� ��� ( ���� �ʿ� )
	float FailureDueToDistance = (HitResult.Distance * (15 / AttackRadius)) / 100;
	FailureRatio += FailureDueToDistance;
	AimingFactor.Add(EAimingFactor::Disatnce, -FailureDueToDistance);

	
	UE_LOG(LogTemp, Warning, L"���� ���� Ȯ�� : %f", 0.8f - FailureRatio);

	return 0.8f - FailureRatio;
}




bool UAimingComponent::GetEnemyInRange(const float AttackRadius, OUT TArray<ACustomThirdPerson*>& CharacterInRange)
{
	TArray<TEnumAsByte<EObjectTypeQuery>> UnUsedObjectType;
	TArray<AActor*> ActorsToIgnore;
	TArray<AActor*> ActorsInRange;

	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetOwner()->GetActorLocation(), AttackRadius, UnUsedObjectType, ACustomThirdPerson::StaticClass(), ActorsToIgnore, ActorsInRange);
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
* ���� ����� �����ϰ��ִ� ���� ���ؼ��� �̷�� ������ ����մϴ�.
* @param AimDirection - ���ؼ��� ���� �����Դϴ�.
* @param TargetPawn - ���� ����� �Ǵ� Pawn�Դϴ�.
* @return ���� ���ؼ��� �̷�� ������ Degree�� ��ȯ�մϴ�.
*/
float UAimingComponent::CalculateAngleBtwAimAndWall(const FVector AimDirection, ACustomThirdPerson* TargetPawn, OUT ECoverInfo& CoverInfo)
{
	float MinAngleBetweenAimAndWall = MAX_FLT;

	FVector North = FVector(0, 1, 0);
	FVector South = FVector(0, -1, 0);
	FVector West = FVector(-1, 0, 0);
	FVector East = FVector(1, 0, 0);

	FString DirectionString;
	FString resultString;

	for (auto CoverDirectionState : TargetPawn->CoverDirectionMap)
	{
		FVector WallForwardVector;
		float AngleBetweenAimAndWall = 0;
		if (CoverDirectionState.Value != ECoverInfo::None)
		{
			switch (CoverDirectionState.Key)
			{
			case ECoverDirection::East:
				WallForwardVector = East;
				DirectionString = "East";
				break;
			case ECoverDirection::West:
				WallForwardVector = West;
				DirectionString = "West";
				break;
			case ECoverDirection::North:
				WallForwardVector = North;
				DirectionString = "North";
				break;
			case ECoverDirection::South:
				WallForwardVector = South;
				DirectionString = "South";
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
				resultString = DirectionString;
			}
		}
	}
	UE_LOG(LogTemp, Warning, L"Minimum Angle : %f , %s", MinAngleBetweenAimAndWall, *resultString);

	//RelativeCoverLoc *= 100;

	return MinAngleBetweenAimAndWall;
}



bool UAimingComponent::FilterAttackableEnemy(const TMap<ECoverDirection, ECoverInfo>& CoverDirectionMap, const TArray<ACustomThirdPerson*>& EnemiesInRange, const bool bIsCovering, OUT TArray<FHitResult>& SensibleEnemyInfo)
{
	TArray<FHitResult> ResultRequireInspection;

	//���� �����϶� �ٰ������� Ȯ���� �ʿ���
	if (bIsCovering)
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
				GetAimingInfoFromSurroundingArea(RightSide, SurroundingAreaInfo);
				GetAimingInfoFromSurroundingArea(LeftSide, SurroundingAreaInfo);

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

	// ����ƴ� ��ġ���� Ȯ��
	for (ACustomThirdPerson* SingleTargetEnemy : EnemiesInRange)
	{
		FHitResult HitResult = LineTraceWhenAiming(GetOwner()->GetActorLocation() , SingleTargetEnemy->GetActorLocation());
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
	GetOwner()->GetActorLocation();
	GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		TargetLocation,
		ECollisionChannel::ECC_GameTraceChannel8,
		CollisionParams
	);
	return HitResult;
}

void UAimingComponent::GetAimingInfoFromSurroundingArea(const FVector SurroundingArea, TArray<FHitResult>&  AimingInfo)
{
	FHitResult HitResultFromCharToSurroundingArea = LineTraceWhenAiming(GetOwner()->GetActorLocation(), SurroundingArea);
	if (!HitResultFromCharToSurroundingArea.GetActor())
	{
		AimingInfo.Add(HitResultFromCharToSurroundingArea);
	}
}

FRotator UAimingComponent::FindCoverDirection(TPair<ECoverDirection, ECoverInfo> DirectionAndInfoPair)
{
	FRotator Direction = FRotator(0, 0, 0);
	switch (DirectionAndInfoPair.Key)
	{
	case ECoverDirection::East:
		Direction = FRotator(0, 0, 0);
		break;
	case ECoverDirection::West:
		Direction = FRotator(0, 180, 0);
		break;
	case ECoverDirection::North:
		Direction = FRotator(0, 90, 0);
		break;
	case ECoverDirection::South:
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

