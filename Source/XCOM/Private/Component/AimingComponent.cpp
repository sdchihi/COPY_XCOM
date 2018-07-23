
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

/**
* ���� ������ ������ ������ ���ɴϴ�.
* @param ActorLocation - ���� ����� �����ϴ� Actor�� ��ġ
* @param AttackRadius - ���� ������ ����
* @param bIsCover - �����ϴ� Actor�� ���� ���� ����
* @param CoverDirectionMap - ���� ���� ������ �����ִ� ��
* @param AimingInfoList - ���� ������ ���� ���� ������ ��ȯ���� Array
*/
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
			TMap<ECriticalFactor, int8> CriticalFactor;

			float Probability = CalculateAttackSuccessRatio(ActorLocation, FilteredHitResult, AttackRadius, bIsCover, DetectedPawn, AimingFactor, CriticalFactor);
			FVector StartLocation = FilteredHitResult.TraceStart;
			FVector TargetLocation = DetectedPawn->GetActorLocation();
			TargetLocationArr.AddUnique(TargetLocation);
			AimingInfoInAllCase.Add(FAimingInfo(StartLocation, TargetLocation, Probability, DetectedPawn, AimingFactor, CriticalFactor));
		}
	}

	FindBestCaseInAimingInfo(AimingInfoInAllCase, AimingInfoList, TargetLocationArr);
	UE_LOG(LogTemp, Warning, L"���� ��� ����� Target �� : %d ", AimingInfoList.Num());
};


/**
* ���� ���� Ȯ���� ����մϴ�.
* @param ActorLocation - ���� ����� �����ϴ� Actor�� ��ġ
* @param HitResult - Linetrace ���
* @param AttackRadius - ���� ������ ����
* @param bIsCover - �����ϴ� Actor�� ���� ���� ����
* @param AimingFactor - ���ؿ� �����ϴ� �ܺ� ��ҿ� ���� �����͸� ��ȯ�մϴ�
* @param CriticalFactor - ũ��Ƽ�ÿ� �����ϴ� �ܺ� ��ҿ� ���� �����͸� ��ȯ�մϴ�.
* @return ���� ���� Ȯ���� ��ȯ�մϴ�.
*/
//���� �ʿ�
float UAimingComponent::CalculateAttackSuccessRatio(const FVector ActorLocation, const FHitResult HitResult, float AttackRadius, const bool bIsCover, APawn* TargetPawn, TMap<EAimingFactor, float>& AimingFactor, TMap<ECriticalFactor, int8>& CriticalFactor)
{
	AimingFactor.Add(EAimingFactor::AimingAbility, 0.8);
	float FailureRatio = 0;
	FVector AimDirection = (ActorLocation - HitResult.GetActor()->GetActorLocation()).GetSafeNormal();
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

	// Todo - ������ �Ÿ��� ���� ���� Ȯ�� ��� ( ���� �ʿ� )
	float FailureDueToDistance = (HitResult.Distance * (15 / AttackRadius)) / 100;
	FailureRatio += FailureDueToDistance;
	AimingFactor.Add(EAimingFactor::Disatnce, -FailureDueToDistance);
	
	UE_LOG(LogTemp, Warning, L"���� ���� ���� Ȯ�� : %f", 0.8f - FailureRatio);

	return 0.8f - FailureRatio;
}


/**
* ���� ���� �ȿ� �ִ� ���� ���ɴϴ�.
* @param ActorLocation - ������ �����ϴ� Actor�� ��ġ
* @param AttackRadius - ���� ���� ����
* @return ���� �ȿ� ���� �ִ��� ���θ� ��ȯ�մϴ�.
*/
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

/**
* ���� ��Ȳ�� �´� LineTrace�� ���� ������ ������ ���͵��� ��ȯ�մϴ�
* @param ActorLocation - ���� Actor�� ��ġ�Դϴ�.
* @param CoverDirectionMap - ���� Actor�� �����ϰ� �ִ� ���󹰿� ���� ������
* @param EnemiesInRange - ���� ���� �ȿ� �����ִ� ���� Array
* @param bIsCovering - ���� Actor�� ���� ����
* @param SensibleEnemyInfo - LineTrace�� ���� Ȯ�ε� ���� Array
* @return ������ �������� ���θ� ��ȯ�մϴ�.
*/
bool UAimingComponent::FilterAttackableEnemy(const FVector ActorLocation, const TMap<EDirection, ECoverInfo>& CoverDirectionMap, const TArray<ACustomThirdPerson*>& EnemiesInRange, const bool bIsCovering, OUT TArray<FHitResult>& SensibleEnemyInfo)
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

	// ����ƴ� ��ġ���� Ȯ��
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

/**
* ���ؿ� �ʿ��� LineTrace�� �����մϴ�.
* @param StartLocation - LineTrace �� ������
* @param TargetLocation - LineTrace �� ����� �Ǵ� Ÿ���� ��ġ
* @return LineTrace ����� ��ȯ�մϴ�.
*/
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

/**
* ������ ���� ���� ��ġ������ LineTrace����� ���ɴϴ�.
* @param ActorLocation - �����ϴ� Actor�� ��ġ
* @param SurroundingArea - ���� ��Ȳ���� ������ ���� ���� �ĺ� ��ġ
* @param AimingInfo - LineTrace�� ����� ��ȯ���� Array
*/
void UAimingComponent::GetAimingInfoFromSurroundingArea(const FVector ActorLocation, const FVector SurroundingArea, TArray<FHitResult>&  AimingInfo)
{
	FHitResult HitResultFromCharToSurroundingArea = LineTraceWhenAiming(ActorLocation, SurroundingArea);
	if (!HitResultFromCharToSurroundingArea.GetActor())
	{
		AimingInfo.Add(HitResultFromCharToSurroundingArea);
	}
}

/**
* ���� ������ Rotator������ ���ɴϴ�.
* @param DirectionAndInfoPair - Map ���� ���� Direction, CoverInfo Pair
* @return ���� ������ Rotator��
*/
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


/**
* ���� �����͸� ���͸��Ͽ� ������ ���� �����͵��� ��ȯ�մϴ�.
* @param AllCaseInfo - ���͸����� ���� Aiming Info Array
* @param BestCaseArr - ���͸� �� ��ȯ��  AimingInfo array
* @param TargetLocArr - Ÿ���� ��ġ Array
*/
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

/**
* ��� �� ���ذ����� ������ ������ ���ɴϴ�.
* @param AttackRadius - ���� ������ ����
* @param bIsCover - ���� Actor�� ���� ����
* @param CoverDirectionMap - �����ϰ� �ִ� ���󹰿� ���� Map
* @return ��� ���� ����
*/
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

			float Probability = CalculateAttackSuccessRatio(ActorLocation, FilteredHitResult, AttackRadius, bIsCover,DetectedPawn, AimingFactor, CriticalFactor);
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

/**
* ������ ���������� ��ȯ�մϴ�..
* @param ActorLocation - ���� Actor�� ��ġ
* @param AttackRadius - ���� ������ ����
* @param bIsCover - ���� Actor�� ���� ����
* @param CoverDirectionMap - �����ϰ� �ִ� ���󹰿� ���� Map
* @return AimingInfo
*/
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


/**
* ���� Actor�� ���ؿ� ����� TraceChannel�� �����ɴϴ�.
* @return ECollisionChannel ���� ä��
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
