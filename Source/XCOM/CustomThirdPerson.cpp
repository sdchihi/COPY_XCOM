// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomThirdPerson.h"
#include "Gun.h"
#include "Classes/Camera/CameraComponent.h"
#include "Classes/GameFramework/SpringArmComponent.h"
#include "Classes/GameFramework/CharacterMovementComponent.h"

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

void ACustomThirdPerson::MoveToLocation(FVector Location) {
	GetCharacterMovement()->MoveSmooth(Location, 2);
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
		GetActorLocation() + GetActorForwardVector() * 50,
		TargetPawn->GetActorLocation(),
		ECollisionChannel::ECC_EngineTraceChannel2,
		CollisionParams
	);


	ACustomThirdPerson* DetectedPawn = Cast<ACustomThirdPerson>(HitResult.GetActor());
	if (DetectedPawn) {
		//DetectedPawn->GunReference->Fire();
		float AttackSuccessRatio = CalculateAttackSuccessRatio(HitResult, TargetPawn);
		GunReference->Fire();
		UE_LOG(LogTemp, Warning, L"Hit Result Actor : %s  , TargetActor : %s  Percentage : %f", *DetectedPawn->GetName(), *TargetPawn->GetName(), AttackSuccessRatio);
	}
}

float ACustomThirdPerson::CalculateAttackSuccessRatio(FHitResult HitResult, APawn* TargetPawn)
{
	float FailureRatio = 0;
	FVector AimDirection = (GetActorLocation() - HitResult.GetActor()->GetActorLocation()).GetSafeNormal();
	
	// ���϶��� �ƴҶ� ������ �ʿ䰡 �ִ�
	// ������ �׽�Ʈ �뵵��.
	FVector WallForwardVector = HitResult.GetActor()->GetActorForwardVector();

	// 0 ~ 180
	float AngleBetweenAimAndWall = FMath::RadiansToDegrees(acosf(FVector::DotProduct(AimDirection, WallForwardVector)));

	if (AngleBetweenAimAndWall < 90) 
	{
		// ��ֹ��� ���� ���� Ȯ�� ���
		FailureRatio += FMath::Cos(FMath::DegreesToRadians(AngleBetweenAimAndWall)) * 0.4;
	}
	// ������ �Ÿ��� ���� ���� Ȯ�� ���
	FailureRatio += (HitResult.Distance * (15 / AttackRange)) / 100;


	return 1 - FailureRatio;
}
