// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomThirdPerson.h"
#include "Gun.h"
#include "Classes/Camera/CameraComponent.h"
#include "Classes/GameFramework/SpringArmComponent.h"
#include "Classes/GameFramework/CharacterMovementComponent.h"
#include "Runtime/Engine/Public/TimerManager.h"
// Sets default values
ACustomThirdPerson::ACustomThirdPerson()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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

	CoverDirectionMap.Add(ECoverDirection::East, false);
	CoverDirectionMap.Add(ECoverDirection::West, false);
	CoverDirectionMap.Add(ECoverDirection::North, false);
	CoverDirectionMap.Add(ECoverDirection::South, false);
	
	GunReference = GetWorld()->SpawnActor<AGun>(
		GunBlueprint,
		FVector(0,0,0),
		FRotator(0, 0, 0)
		);;
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



void ACustomThirdPerson::CheckAttackPotential(APawn* TargetPawn) 
{
	FCollisionQueryParams CollisionQueryParam;
	CollisionQueryParam;
	FCollisionResponseParams CollisionResponseParam;

	const FName TraceTag("MyTraceTag");

	GetWorld()->DebugDrawTraceTag = TraceTag;

	FCollisionQueryParams CollisionParams;
	CollisionParams.TraceTag = TraceTag;


	//GetActorLocation() +GetActorForwardVector() * 50

	FHitResult HitResult;
	GetActorLocation();
	GetWorld()->LineTraceSingleByChannel(
		HitResult,
		GetActorLocation(),// + GetActorForwardVector() * 50,
		TargetPawn->GetActorLocation(),
		ECollisionChannel::ECC_EngineTraceChannel2,
		CollisionParams
	);


	ACustomThirdPerson* DetectedPawn = Cast<ACustomThirdPerson>(HitResult.GetActor());
	if (DetectedPawn) {
		//DetectedPawn->GunReference->Fire();
		float AttackSuccessRatio = CalculateAttackSuccessRatio(HitResult, TargetPawn);
		GunReference->Fire();
		bIsAttack = true;

		bCanAction = false;
		UE_LOG(LogTemp, Warning, L"Hit Result Actor : %s  , TargetActor : %s  Success  Percentage : %f", *DetectedPawn->GetName(), *TargetPawn->GetName(), AttackSuccessRatio);

		FTimerHandle UnUsedHandle;
		FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ACustomThirdPerson::SetOffAttackState);
		GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, 1.4, false);
	}
}

float ACustomThirdPerson::CalculateAttackSuccessRatio(FHitResult HitResult, APawn* TargetPawn)
{
	float FailureRatio = 0;
	FVector AimDirection = (GetActorLocation() - HitResult.GetActor()->GetActorLocation()).GetSafeNormal();
	ACustomThirdPerson* TargetThirdPerson = Cast<ACustomThirdPerson>(TargetPawn);
	
	// 클래스가 유연하지 않으므로 이후 수정
	if (!TargetThirdPerson) {
		UE_LOG(LogTemp, Warning, L"Fucked");

		return 0; 
	}

	
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
			switch (CoverDirectionState.Key) {
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
			
			if (MinAngleBetweenAimAndWall > AngleBetweenAimAndWall) {
				MinAngleBetweenAimAndWall = AngleBetweenAimAndWall;
				resultString = DirectionString;
			}
		}

	}

	UE_LOG(LogTemp, Warning, L"Minimum Angle : %f , %s", MinAngleBetweenAimAndWall, *resultString);

	return MinAngleBetweenAimAndWall;
}

// 엄폐 상황에서 애니메이션을 위한 함수
void ACustomThirdPerson::RotateTowardWall() {
	for (auto CoverDirection : CoverDirectionMap) {
		if (CoverDirection.Value == true) {
			FRotator Direction;
			switch (CoverDirection.Key) {
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

void ACustomThirdPerson::SetOffAttackState() {
	bIsAttack = false;
}