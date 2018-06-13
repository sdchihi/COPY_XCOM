// Fill out your copyright notice in the Description page of Project Settings.

#include "CoveringChecker.h"
#include "Components/InstancedStaticMeshComponent.h"

ACoveringChecker::ACoveringChecker()
{
	//PrimaryActorTick.bCanEverTick = true;
	InstancedStaticMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(FName("InstancedStaticMesh"));
	InstancedStaticMesh->RegisterComponent();
	InstancedStaticMesh->SetFlags(RF_Transactional);
	this->AddInstanceComponent(InstancedStaticMesh);

}

void ACoveringChecker::BeginPlay()
{
	Super::BeginPlay();
	if (MeshToRegister) 
	{
		InstancedStaticMesh->SetStaticMesh(MeshToRegister);
	}
}

void ACoveringChecker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACoveringChecker::MakingCoverNotice(TArray<FVector>& TileLocationArray, float Spacing) const
{

	TArray<FTransform> CoverNoticeTF;
	for (FVector TileLocation : TileLocationArray) 
	{
		TArray<FTransform> CoverNoticeSegment;
		CoverNoticeSegment = RayCastToCardinalDirection(TileLocation, Spacing);
		CoverNoticeTF.Append(CoverNoticeSegment);
	}
	UE_LOG(LogTemp, Warning, L"%d개 위치 생성", CoverNoticeTF.Num());


	AddInstances(CoverNoticeTF);
}

TArray<FTransform> ACoveringChecker::RayCastToCardinalDirection(FVector OriginLocation, float Spacing) const 
{
	TArray<FTransform> ShieldTransformArray;
	FVector CardinalLocation[4] = { 
		OriginLocation + FVector(Spacing, 0, 0),
		OriginLocation + FVector(-Spacing, 0, 0),
		OriginLocation + FVector(0, Spacing, 0),
		OriginLocation + FVector(0, -Spacing, 0)
		};

	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams CollisionParams;
	CollisionParams.TraceTag = TraceTag;
	CollisionParams.bFindInitialOverlaps = false;

	for (int i = 0; i < 4; i++) 
	{
		FHitResult HitResult;
		GetWorld()->LineTraceSingleByChannel(
			HitResult,
			OriginLocation,
			CardinalLocation[i],
			ECC_GameTraceChannel12,
			CollisionParams
		);

		if (HitResult.GetActor()) 
		{
			FTransform ShieldTransform;
			ShieldTransform.SetLocation(HitResult.ImpactPoint);
			ShieldTransform.SetRotation((-HitResult.ImpactNormal).ToOrientationQuat());
			ShieldTransform.SetScale3D(FVector(1, 1, 1));
			
			UE_LOG(LogTemp, Warning, L"여기 충돌감지됬서요");
			ShieldTransformArray.Add(ShieldTransform);
		}

	}

	return ShieldTransformArray;
}


void ACoveringChecker::AddInstances(TArray<FTransform>& TransformArrray) const 
{
	for (FTransform SingleTransform : TransformArrray) 
	{
		InstancedStaticMesh->AddInstanceWorldSpace(SingleTransform);
		UE_LOG(LogTemp, Warning, L"임시 : Instance 생성 디버깅 로그");
	}
}

void ACoveringChecker::ClearAllCoverNotice()
{
	InstancedStaticMesh->ClearInstances();
}