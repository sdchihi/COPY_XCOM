// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomThirdPerson.h"
#include "Gun.h"
#include "Classes/Camera/CameraComponent.h"
#include "Classes/Components/SkeletalMeshComponent.h"
#include "Classes/GameFramework/SpringArmComponent.h"
#include "Classes/GameFramework/CharacterMovementComponent.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "Classes/Kismet/KismetSystemLibrary.h"

// Sets default values
ACustomThirdPerson::ACustomThirdPerson()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 700.0f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Follow Camera"));
	FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
}

void ACustomThirdPerson::BeginPlay()
{
	Super::BeginPlay();

	CurrentMovableStep = Step;

	CoverDirectionMap.Add(ECoverDirection::East, ECoverInfo::None);
	CoverDirectionMap.Add(ECoverDirection::West, ECoverInfo::None);
	CoverDirectionMap.Add(ECoverDirection::North, ECoverInfo::None);
	CoverDirectionMap.Add(ECoverDirection::South, ECoverInfo::None);
	
	GunReference = GetWorld()->SpawnActor<AGun>(
		GunBlueprint,
		FVector(0,0,0),
		FRotator(0, 0, 0)
		);

	USkeletalMeshComponent* Mesh = FindComponentByClass<USkeletalMeshComponent>();
	if (Mesh) 
	{
		GunReference->AttachToComponent(Cast<USceneComponent>(Mesh), FAttachmentTransformRules::KeepRelativeTransform,  FName(L"Gun"));
	}
}
 
void ACustomThirdPerson::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

	
// Called to bind functionality to input
void ACustomThirdPerson::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}


void ACustomThirdPerson::ClearCoverDirectionInfo() 
{
	// Add �޼ҵ尡 �ߺ��� Key�������ؼ� ���� ������ ��.
	CoverDirectionMap.Add(ECoverDirection::East, ECoverInfo::None);
	CoverDirectionMap.Add(ECoverDirection::West, ECoverInfo::None);
	CoverDirectionMap.Add(ECoverDirection::North, ECoverInfo::None);
	CoverDirectionMap.Add(ECoverDirection::South, ECoverInfo::None);

	bIsCovering = false;
}


/**
* ����� ������ �� �ִ��� Ȯ���ϰ�, ������ �����ϴٸ� ���� Ȯ���� ����س��ϴ�.
* @param TargetPawn - ���� ����̵Ǵ� Pawn �Դϴ�.
*/
void ACustomThirdPerson::CheckAttackPotential(APawn* TargetPawn) 
{
	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams CollisionParams;
	CollisionParams.TraceTag = TraceTag;

	FHitResult HitResult;
	GetActorLocation();
	GetWorld()->LineTraceSingleByChannel(
		HitResult,
		GetActorLocation() +GetActorUpVector() * 110,
		TargetPawn->GetActorLocation(),
		ECollisionChannel::ECC_GameTraceChannel8,
		CollisionParams
	);

	ACustomThirdPerson* DetectedPawn = Cast<ACustomThirdPerson>(HitResult.GetActor());
	if (DetectedPawn) 
	{
		float AttackSuccessRatio = CalculateAttackSuccessRatio(HitResult, DetectedPawn);
		FVector AimDirection = HitResult.Location - GetActorLocation();

		float RandomValue = FMath::RandRange(0, 1);
		//����
		if (RandomValue < AttackSuccessRatio) 
		{
			FVector AimDirection = HitResult.Location - GetActorLocation();
			AimAt(AimDirection, FName("ProjectileToChar"));
			UE_LOG(LogTemp, Warning, L"���� ����");

		}
		else 
		{
			if (DetectedPawn->bIsCovering) 
			{
				RandomValue = FMath::RandRange(0, 1);
				if (RandomValue < 0.5) // ��� ����
				{
					AimAt(AimDirection + FVector(0, 70, 150), FName("ProjectileToWall"));
					UE_LOG(LogTemp, Warning, L"���� : ������� ���");
				}
				else				// �� ����
				{
					AimAt(AimDirection + RelativeCoverLoc, FName("ProjectileToWall"));
					UE_LOG(LogTemp, Warning, L"���� : ���� ���� ���� ���");
				}
			}
			else 
			{
				AimAt(AimDirection + FVector(0, 20, 50), FName("ProjectileToWall"));
				UE_LOG(LogTemp, Warning, L"���� : ���� ���� ���� ���");
			}
		}
	}
}

/**
* ���� ���� Ȯ���� ����մϴ�.
* @param HitResult - ���� ����� ���� RayCast ��� ����� ����Դϴ�.
* @param TargetPawn - ���� ����� �Ǵ� Pawn�Դϴ�.
* @return ���� ���� Ȯ���� ��ȯ�մϴ�.
*/
float ACustomThirdPerson::CalculateAttackSuccessRatio(const FHitResult HitResult, APawn* TargetPawn)
{
	float FailureRatio = 0;
	FVector AimDirection = (GetActorLocation() - HitResult.GetActor()->GetActorLocation()).GetSafeNormal();
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
			}
			else if (CoverInfo == ECoverInfo::FullCover) 
			{
				FailureDueToCover = 0.4;
			}
			FailureRatio += FailureDueToCover;
			UE_LOG(LogTemp, Warning, L"����� ���� ���� Ȯ�� ��� ��� : %f", FailureDueToCover);
		}
	}
	// Todo - ������ �Ÿ��� ���� ���� Ȯ�� ��� ( ���� �ʿ� )
	FailureRatio += (HitResult.Distance * (15 / AttackRange)) / 100;

	UE_LOG(LogTemp, Warning, L"Distance : %f, �Ÿ��� ���� ���� Ȯ�� : %f , ���� Ȯ�� : %f", HitResult.Distance, (HitResult.Distance * (15 / AttackRange)) / 100, FailureRatio);

	return 1 - FailureRatio;
}

/**
* ���� ����� �����ϰ��ִ� ���� ���ؼ��� �̷�� ������ ����մϴ�.
* @param AimDirection - ���ؼ��� ���� �����Դϴ�.
* @param TargetPawn - ���� ����� �Ǵ� Pawn�Դϴ�.
* @return ���� ���ؼ��� �̷�� ������ Degree�� ��ȯ�մϴ�.
*/
float ACustomThirdPerson::CalculateAngleBtwAimAndWall(const FVector AimDirection, ACustomThirdPerson* TargetPawn, OUT ECoverInfo& CoverInfo)
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
				RelativeCoverLoc = WallForwardVector;
				MinAngleBetweenAimAndWall = AngleBetweenAimAndWall;
				resultString = DirectionString;
			}
		}
	}
	UE_LOG(LogTemp, Warning, L"Minimum Angle : %f , %s", MinAngleBetweenAimAndWall, *resultString);

	RelativeCoverLoc *= 100;

	return MinAngleBetweenAimAndWall;
}

/**
* ����� �ùٸ� �ִϸ��̼��� ���� ���� ���� ȸ���մϴ�.
*/
void ACustomThirdPerson::RotateTowardWall() {

	for (auto CoverDirection : CoverDirectionMap)
	{
		if (CoverDirection.Value != ECoverInfo::None)
		{
			FRotator Direction;
			switch (CoverDirection.Key)
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
			SetActorRotation(Direction);
		}
	}
}

/**
* ������ ���� �� ��������Ʈ ����, Flag ��ȯ
*/
void ACustomThirdPerson::SetOffAttackState() {
	bIsAttack = false;
	if (bIsCovering) {
		RotateTowardWall();
	}
	if (ChangePlayerPawnDelegate.IsBound()) 
	{
		ChangePlayerPawnDelegate.Execute();
	}
	else 
	{
		UE_LOG(LogTemp, Warning, L"Unbound");
	}
}

/**
* �������Ʈ���� ȣ��� �� �ִ� �޼ҵ�� ���� �ð����� ĳ���͸� ��ǥ �������� ȸ����ŵ�ϴ�. (Timeline�� �����ؼ� ���)
* @param AimDirection - ��ǥ ���� �����Դϴ�.
* @param LerpAlpha - Lerp�� ���� Alpha���Դϴ�.
*/
void ACustomThirdPerson::RotateCharacter(FVector AimDirection, float LerpAlpha) 
{
	float CharacterRotatorYaw = GetActorRotation().Yaw;
	float AimAsRotatorYaw = AimDirection.Rotation().Yaw;

	SetActorRotation(FRotator(0,FMath::Lerp(CharacterRotatorYaw, AimAsRotatorYaw, LerpAlpha), 0));
}

void ACustomThirdPerson::StartFiring(FName NewCollisionPresetName)
{
	bIsAttack = true;
	GunReference->ProjectileCollisionPresetName = NewCollisionPresetName;
	UseActionPoint(2);

	FTimerHandle UnUsedHandle;
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ACustomThirdPerson::SetOffAttackState);
	GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 1.4 + 1.5, false);
}

void ACustomThirdPerson::UseActionPoint(int32 PointToUse) 
{
	RemainingActionPoint -= PointToUse;
	UE_LOG(LogTemp, Warning, L"Use %d Action Point  --  Remaining : %d", PointToUse, RemainingActionPoint);
	if (RemainingActionPoint <=0) 
	{
		bCanAction = false;
		if (CheckTurnDelegate.IsBound())
		{
			CheckTurnDelegate.Execute(bTeam);
		}
	}
}

float ACustomThirdPerson::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) 
{
	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	CurrentHP -= ActualDamage;
	if (CurrentHP <= 0) 
	{
		//TODO ��� Event
		UE_LOG(LogTemp, Warning, L"Dead");
	}

	return ActualDamage;
}

void ACustomThirdPerson::RestoreActionPoint()
{
	bCanAction = true;
	RemainingActionPoint = 2;
}

void ACustomThirdPerson::GetAttackableEnemyInfo() 
{
	TArray<ACustomThirdPerson*> EnemyInRange;
	if (GetEnemyInRange(EnemyInRange) == false) 
	{
		return;
	};

	TArray<FHitResult> AttackableEnemysHitResult;
	if (FilterAttackableEnemy(EnemyInRange, AttackableEnemysHitResult) == false)
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
			float Probability = CalculateAttackSuccessRatio(FilteredHitResult, DetectedPawn);
			FVector StartLocation = FilteredHitResult.TraceStart;
			FVector TargetLocation = DetectedPawn->GetActorLocation();
			TargetLocationArr.AddUnique(TargetLocation);
			AimingInfoInAllCase.Add(FAimingInfo(StartLocation, TargetLocation, Probability));
		}
	}
	
	FindBestCaseInAimingInfo(AimingInfoInAllCase, AimingInfoList, TargetLocationArr);
	UE_LOG(LogTemp, Warning, L"���� ��� ����� Target �� : %d ", AimingInfoList.Num());
	
};

bool ACustomThirdPerson::GetEnemyInRange(OUT TArray<ACustomThirdPerson*>& CharacterInRange) 
{
	TArray<TEnumAsByte<EObjectTypeQuery>> UnUsedObjectType;
	TArray<AActor*> ActorsToIgnore;
	TArray<AActor*> ActorsInRange;

	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation(), AttackRadius, UnUsedObjectType, ACustomThirdPerson::StaticClass(), ActorsToIgnore, ActorsInRange);
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

bool ACustomThirdPerson::FilterAttackableEnemy(const TArray<ACustomThirdPerson*>& EnemiesInRange, OUT TArray<FHitResult>& SensibleEnemyInfo)
{
	TArray<FHitResult> ResultRequireInspection;

	//���� �����϶� �ٰ������� Ȯ���� �ʿ���
	if (this->bIsCovering) 
	{
		for (auto CoverDirection : CoverDirectionMap)
		{
			if (CoverDirection.Value != ECoverInfo::None)
			{
				FRotator Direction = FindCoverDirection(CoverDirection);

				FVector RightVector = FVector::CrossProduct(FVector::UpVector, Direction.Vector());
				FVector RightSide = GetActorLocation() + RightVector * 100;
				FVector LeftSide = GetActorLocation() - RightVector * 100;

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
		FHitResult HitResult = LineTraceWhenAiming(GetActorLocation() /*+ GetActorUpVector() * 110*/ , SingleTargetEnemy->GetActorLocation());
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


FHitResult ACustomThirdPerson::LineTraceWhenAiming(const FVector StartLocation, const FVector TargetLocation)
{
	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams CollisionParams;
	CollisionParams.TraceTag = TraceTag;
	CollisionParams.bFindInitialOverlaps = false;

	FHitResult HitResult;
	GetActorLocation();
	GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		TargetLocation,
		ECollisionChannel::ECC_GameTraceChannel8,
		CollisionParams
	);

	return HitResult;
}

void ACustomThirdPerson::GetAimingInfoFromSurroundingArea(const FVector SurroundingArea, TArray<FHitResult>&  AimingInfo)
{
	FHitResult HitResultFromCharToSurroundingArea = LineTraceWhenAiming(GetActorLocation(), SurroundingArea);
	if (!HitResultFromCharToSurroundingArea.GetActor()) 
	{
		AimingInfo.Add(HitResultFromCharToSurroundingArea);
	}
}

FRotator ACustomThirdPerson::FindCoverDirection(TPair<ECoverDirection,ECoverInfo> DirectionAndInfoPair)
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


void ACustomThirdPerson::FindBestCaseInAimingInfo(const TArray<FAimingInfo> AllCaseInfo, TArray<FAimingInfo>& BestCaseArr, const TArray<FVector> TargetLocArr) 
{
	for (FVector TargetLoc : TargetLocArr) 
	{
		FAimingInfo SingleBestCase;
		float HighestProbability = 0;
		for (FAimingInfo SingleCaseInCheck : AllCaseInfo)
		{
			if (HighestProbability < SingleCaseInCheck.Probability)
			{
				SingleBestCase = SingleCaseInCheck;
			}
		}
		BestCaseArr.Add(SingleBestCase);
	}
}

