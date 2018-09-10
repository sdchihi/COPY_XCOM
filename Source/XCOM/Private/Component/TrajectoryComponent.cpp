// Fill out your copyright notice in the Description page of Project Settings.

#include "TrajectoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CustomThirdPerson.h"

UTrajectoryComponent::UTrajectoryComponent() 
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UTrajectoryComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bIsDrawing) 
	{
		DrawProjectileTrajectory();
	}
};

/**
* 투사체의 궤적을 그려냅니다.
* @param bCanCognize 안개안에 위치하고있는지 여부
*/
void UTrajectoryComponent::DrawProjectileTrajectory() 
{
	FVector ActorLocation = GetOwner()->GetActorLocation();;
	APlayerController* PlayerController= UGameplayStatics::GetPlayerController(GetWorld(), 0);
	FHitResult MousePointHitResult;
	PlayerController->GetHitResultUnderCursor(ECollisionChannel::ECC_MAX, true, MousePointHitResult);

	float DistanceFromCharToPoint = FVector::Distance(MousePointHitResult.ImpactPoint , ActorLocation);
	float Power = FMath::Clamp(DistanceFromCharToPoint , 0.f, MaxThrowingPower);
	float CorrectedPower = CorrectPower(Power);
	//UE_LOG(LogTemp, Warning, L"거리 : %f   힘 : %f ", DistanceFromCharToPoint, CorrectedPower);
	InitialLocalVelocity = FVector(Power, 0, Power);
	//수정 필요
	FRotator RotationToTarget = (MousePointHitResult.ImpactPoint - ActorLocation).Rotation();
	InitialVelocity = FTransform(RotationToTarget, ActorLocation).TransformVector(InitialLocalVelocity);

	/*
	if (ImpactRangeActor)
	{
		ImpactRangeActor->SetActorLocation(MousePointHitResult.ImpactPoint);
	}
	*/
	
	int32 PointCount= FMath::FloorToInt(PathLifeTime / TimeInterval);
	
	FVector GravityVector = FVector(0, 0, -980.0);
	for (int i = 0; i < PointCount; i++) 
	{
		float Time1 = i * TimeInterval;
		float Time2 = (i + 1) * TimeInterval;

		FVector FirstPoint;
		FVector SecondPoint;
		GetSegmentAtTime(ActorLocation, InitialVelocity, GravityVector, Time1, Time2, FirstPoint, SecondPoint);
		
		FHitResult HitResult;
		TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjects;
		TArray<AActor*> Filter;
		Filter.Add(ImpactRangeActor);
		TraceObjects.Add(UEngineTypes::ConvertToObjectType(ECC_Visibility));//add your object types here.
		UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), FirstPoint, SecondPoint, TraceObjects, true, Filter, EDrawDebugTrace::ForOneFrame, HitResult, true);

		if (HitResult.GetActor()) 
		{
			ImpactRangeActor->SetActorLocation(SecondPoint);
			break;
		}

	}
}

/**
* 투사체의 궤적을 그려냅니다.
* @param StartLocation 그려낼 궤적의 시작 지점
* @param InitialVelocity 속도 벡터
* @param Gravity 중력 벡터
* @param Time 
* @param Time2 
* @param FirstPoint 궤도를 그릴 첫번째 점 위치
* @param SecondPoint 궤도를 그릴 두번째 점 위치
*/
void UTrajectoryComponent::GetSegmentAtTime(const FVector StartLocation, const FVector InitialVelocity, const FVector Gravity, const float Time, const float Time2, OUT FVector& FirstPoint, OUT FVector& SecondPoint)
{
	// 시간에 따른 투사체의 위치 
	// ProjectileLocation = StartLoc + InitialVelocity * Time + ( Time * Time * 0.5 ) * Gravity 
	FVector CorrectedVelocity = InitialVelocity * Time;
	FVector GravityAppliedValue =  Gravity * (Time * Time * 0.5);
	FirstPoint = StartLocation + CorrectedVelocity + GravityAppliedValue;

	CorrectedVelocity = InitialVelocity * Time2;
	GravityAppliedValue = Gravity * (Time2 * Time2 * 0.5);
	SecondPoint = StartLocation + CorrectedVelocity + GravityAppliedValue;
};

/**
* 궤도 추적을 시작합니다.
*/
void UTrajectoryComponent::StartDraw() 
{
	SpawnImapactRangeActor();

	bIsDrawing = true;
};

/**
* 궤도 추적을 종료합니다.
*/
void UTrajectoryComponent::FinishDraw() 
{
	if (!ImpactRangeActor) 
	{
		return;
	}
	ImpactRangeActor->Destroy();
	ImpactRangeActor = nullptr;

	bIsDrawing = false;
};

float UTrajectoryComponent::CorrectPower(const float Power)
{
	float Multilplier = FMath::DegreesToRadians(90.f / MaxThrowingPower *Power);
	float CorrectedPower = (FMath::Cos(Multilplier)*0.2 + 1) *Power;
	//UE_LOG(LogTemp, Warning, L"corrected : %f ", (FMath::Cos(Multilplier)*0.2));
	return CorrectedPower;
}

/**
* 궤도 추적의 도착점에 투사체 폭발 반경을 나타낼 Actor를 생성합니다.
*/
void UTrajectoryComponent::SpawnImapactRangeActor() 
{
	if (ImpactRangeBP) 
	{
		ImpactRangeActor = GetWorld()->SpawnActor<AActor>(
			ImpactRangeBP,
			FVector(0, 0, 0),
			FRotator(0, 0, 0)
			);
		USphereComponent* SphereCollision = ImpactRangeActor->FindComponentByClass<USphereComponent>();
		UStaticMeshComponent* Mesh = ImpactRangeActor->FindComponentByClass<UStaticMeshComponent>();
		if (SphereCollision) 
		{
			//SphereCollision->bGenerateOverlapEvents = true;
			SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &UTrajectoryComponent::OnOverlapBegin);
			SphereCollision->OnComponentEndOverlap.AddDynamic(this, &UTrajectoryComponent::EndOverlap);
			SphereCollision->OnClicked.AddUniqueDynamic(this, &UTrajectoryComponent::ConfirmedExplosionArea);
			
			Mesh->OnClicked.AddDynamic(this, &UTrajectoryComponent::ConfirmedExplosionArea);
		}
	}
}

void UTrajectoryComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		//UE_LOG(LogTemp, Warning, L"%s is in imapact range", *OtherActor->GetName());
	}
}

void UTrajectoryComponent::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		//UE_LOG(LogTemp, Warning, L"%s is in imapact range", *OtherActor->GetName());
	}
}

void UTrajectoryComponent::ConfirmedExplosionArea(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	FinishDraw();
	UE_LOG(LogTemp, Warning, L"수류타안 발사");
	ACustomThirdPerson* Owner = Cast<ACustomThirdPerson>(GetOwner());
	if (Owner) 
	{
		Owner->PrepareThrowGrenade(InitialVelocity);
	}
}
