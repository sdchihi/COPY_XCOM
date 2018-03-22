// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomThirdPerson.h"
#include "Gun.h"
#include "Classes/Camera/CameraComponent.h"
#include "Classes/Components/SkeletalMeshComponent.h"
#include "Classes/GameFramework/SpringArmComponent.h"
#include "Classes/GameFramework/CharacterMovementComponent.h"
#include "Runtime/Engine/Public/TimerManager.h"
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

	CoverDirectionMap.Add(ECoverDirection::East, false);
	CoverDirectionMap.Add(ECoverDirection::West, false);
	CoverDirectionMap.Add(ECoverDirection::North, false);
	CoverDirectionMap.Add(ECoverDirection::South, false);
	
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
	// Add 메소드가 중복된 Key값에대해선 갱신 역할을 함.
	CoverDirectionMap.Add(ECoverDirection::East, false);
	CoverDirectionMap.Add(ECoverDirection::West, false);
	CoverDirectionMap.Add(ECoverDirection::North, false);
	CoverDirectionMap.Add(ECoverDirection::South, false);

	bIsCovering = false;
}


/**
* 대상을 공격할 수 있는지 확인하고, 공격이 가능하다면 공격 확률을 계산해냅니다.
* @param TargetPawn - 공격 대상이되는 Pawn 입니다.
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
		ECollisionChannel::ECC_EngineTraceChannel2,
		CollisionParams
	);

	ACustomThirdPerson* DetectedPawn = Cast<ACustomThirdPerson>(HitResult.GetActor());
	if (DetectedPawn) 
	{
		float AttackSuccessRatio = CalculateAttackSuccessRatio(HitResult, TargetPawn);
		FVector AimDirection = HitResult.Location - GetActorLocation();

		//GunReference->Fire();
		
		AimAt(AimDirection);
	}
}

/**
* 공격 성공 확률을 계산합니다.
* @param HitResult - 공격 대상을 향해 RayCast 결과 얻어진 결과입니다.
* @param TargetPawn - 공격 대상이 되는 Pawn입니다.
* @return 공격 성공 확률을 반환합니다.
*/
float ACustomThirdPerson::CalculateAttackSuccessRatio(const FHitResult HitResult, APawn* TargetPawn)
{
	float FailureRatio = 0;
	FVector AimDirection = (GetActorLocation() - HitResult.GetActor()->GetActorLocation()).GetSafeNormal();
	ACustomThirdPerson* TargetThirdPerson = Cast<ACustomThirdPerson>(TargetPawn);
	
	// 클래스가 유연하지 않으므로 이후 수정
	if (!TargetThirdPerson) { return 0; }
	
	//엄폐 상태 확인 수정
	if (TargetThirdPerson->bIsCovering)
	{
		float AngleBetweenAimAndWall = 0;
		AngleBetweenAimAndWall = CalculateAngleBtwAimAndWall(AimDirection, TargetThirdPerson);
		if (AngleBetweenAimAndWall < 90)
		{
			// 장애물에 의한 실패 확률 상승
			FailureRatio += FMath::Cos(FMath::DegreesToRadians(AngleBetweenAimAndWall)) * 0.4;
		}
	}
	// 적과의 거리에 따른 실패 확률 상승
	FailureRatio += (HitResult.Distance * (15 / AttackRange)) / 100;

	UE_LOG(LogTemp, Warning, L"Distance : %f, 거리에 의한 실패 확률 : %f , 성공 확률 : %f" ,HitResult.Distance, (HitResult.Distance * (15 / AttackRange)) / 100, FailureRatio);

	return 1 - FailureRatio;
}


/**
* 공격 대상이 엄폐하고있는 벽과 조준선이 이루는 각도를 계산합니다.
* @param AimDirection - 조준선의 방향 벡터입니다.
* @param TargetPawn - 공격 대상이 되는 Pawn입니다.
* @return 벽과 조준선이 이루는 각도를 Degree로 반환합니다.
*/
float ACustomThirdPerson::CalculateAngleBtwAimAndWall(const FVector AimDirection,ACustomThirdPerson* TargetPawn)
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
		if (CoverDirectionState.Value == true)
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
				MinAngleBetweenAimAndWall = AngleBetweenAimAndWall;
				resultString = DirectionString;
			}
		}
	}
	UE_LOG(LogTemp, Warning, L"Minimum Angle : %f , %s", MinAngleBetweenAimAndWall, *resultString);

	return MinAngleBetweenAimAndWall;
}

/**
* 엄폐시 올바른 애니메이션을 위해 벽을 향해 회전합니다.
*/
void ACustomThirdPerson::RotateTowardWall() {
	for (auto CoverDirection : CoverDirectionMap) 
	{
		if (CoverDirection.Value == true) 
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
			return;
		}
	}
}


/**
* 공격이 끝난 후 델리게이트 실행, Flag 변환
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
* 블루프린트에서 호출될 수 있는 메소드로 일정 시간내에 캐릭터를 목표 방향으로 회전시킵니다. (Timeline과 연계해서 사용)
* @param AimDirection - 목표 방향 벡터입니다.
* @param LerpAlpha - Lerp에 사용될 Alpha값입니다.
*/
void ACustomThirdPerson::RotateCharacter(FVector AimDirection, float LerpAlpha) 
{
	float CharacterRotatorYaw = GetActorRotation().Yaw;
	float AimAsRotatorYaw = AimDirection.Rotation().Yaw;

	SetActorRotation(FRotator(0,FMath::Lerp(CharacterRotatorYaw, AimAsRotatorYaw, LerpAlpha), 0));
}

void ACustomThirdPerson::StartFiring() 
{
	bIsAttack = true;
	
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
	}
}