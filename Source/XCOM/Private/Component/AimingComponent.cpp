
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
* 조준 가능한 적들의 정보를 얻어옵니다.
* @param ActorLocation - 조준 기능을 실행하는 Actor의 위치
* @param AttackRadius - 조준 가능한 범위
* @param bIsCover - 실행하는 Actor가 엄폐 상태 여부
* @param CoverDirectionMap - 엄폐에 대한 정보를 갖고있는 맵
* @param AimingInfoList - 조준 가능한 적에 대한 정보를 반환해줄 Array
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
	UE_LOG(LogTemp, Warning, L"감지 결과 검출된 Target 수 : %d ", AimingInfoList.Num());
};


/**
* 공격 성공 확률을 계산합니다.
* @param ActorLocation - 조준 기능을 실행하는 Actor의 위치
* @param HitResult - Linetrace 결과
* @param AttackRadius - 조준 가능한 범위
* @param bIsCover - 실행하는 Actor가 엄폐 상태 여부
* @param AimingFactor - 조준에 관여하는 외부 요소에 대한 데이터를 반환합니다
* @param CriticalFactor - 크리티컬에 관여하는 외부 요소에 대한 데이터를 반환합니다.
* @return 공격 성공 확률을 반환합니다.
*/
//수정 필요
float UAimingComponent::CalculateAttackSuccessRatio(const FVector ActorLocation, const FHitResult HitResult, float AttackRadius, const bool bIsCover, APawn* TargetPawn, TMap<EAimingFactor, float>& AimingFactor, TMap<ECriticalFactor, int8>& CriticalFactor)
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
	float FailureDueToDistance = (HitResult.Distance * (15 / AttackRadius)) / 100;
	FailureRatio += FailureDueToDistance;
	AimingFactor.Add(EAimingFactor::Disatnce, -FailureDueToDistance);
	
	UE_LOG(LogTemp, Warning, L"최종 공격 성공 확률 : %f", 0.8f - FailureRatio);

	return 0.8f - FailureRatio;
}


/**
* 공격 범위 안에 있는 적을 얻어옵니다.
* @param ActorLocation - 조준을 실행하는 Actor의 위치
* @param AttackRadius - 공격 가능 범위
* @return 범위 안에 적이 있는지 여부를 반환합니다.
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

/**
* 엄폐 상황에 맞는 LineTrace를 통해 조준이 가능한 액터들을 반환합니다
* @param ActorLocation - 현재 Actor의 위치입니다.
* @param CoverDirectionMap - 현재 Actor가 엄폐하고 있는 엄폐물에 대한 데이터
* @param EnemiesInRange - 공격 범위 안에 들어와있는 적의 Array
* @param bIsCovering - 현재 Actor의 엄폐 여부
* @param SensibleEnemyInfo - LineTrace를 통해 확인된 적의 Array
* @return 조준이 가능한지 여부를 반환합니다.
*/
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

/**
* 조준에 필요한 LineTrace를 수행합니다.
* @param StartLocation - LineTrace 의 시작점
* @param TargetLocation - LineTrace 의 대상이 되는 타겟의 위치
* @return LineTrace 결과를 반환합니다.
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
* 엄폐중 공격 가능 위치에서의 LineTrace결과를 얻어옵니다.
* @param ActorLocation - 조준하는 Actor의 위치
* @param SurroundingArea - 엄폐 상황에서 가능한 공격 시작 후보 위치
* @param AimingInfo - LineTrace의 결과를 반환받을 Array
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
* 엄폐 방향을 Rotator값으로 얻어옵니다.
* @param DirectionAndInfoPair - Map 에서 얻어온 Direction, CoverInfo Pair
* @return 엄폐 방향의 Rotator값
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
* 조준 데이터를 필터링하여 최적의 조준 데이터들을 반환합니다.
* @param AllCaseInfo - 필터링되지 않은 Aiming Info Array
* @param BestCaseArr - 필터링 후 반환할  AimingInfo array
* @param TargetLocArr - 타겟의 위치 Array
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
* 경계 중 조준가능한 적들의 정보를 얻어옵니다.
* @param AttackRadius - 조준 가능한 범위
* @param bIsCover - 현재 Actor의 엄폐 여부
* @param CoverDirectionMap - 엄폐하고 있는 엄폐물에 대한 Map
* @return 사격 가능 여부
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
* 최적의 조준정보를 반환합니다..
* @param ActorLocation - 현재 Actor의 위치
* @param AttackRadius - 공격 가능한 범위
* @param bIsCover - 현재 Actor의 엄폐 여부
* @param CoverDirectionMap - 엄폐하고 있는 엄폐물에 대한 Map
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
