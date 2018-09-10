// Fill out your copyright notice in the Description page of Project Settings.

#include "CoveringChecker.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Classes/Materials/MaterialInstanceDynamic.h"
#include "Classes/Engine/StaticMesh.h"
#include "DestructibleWall.h"


ACoveringChecker::ACoveringChecker()
{
	//PrimaryActorTick.bCanEverTick = true;
	FullCoverInstancedMeshComp = CreateDefaultSubobject<UInstancedStaticMeshComponent>(FName("FullCoverInstancedMesh"));
	FullCoverInstancedMeshComp->RegisterComponent();
	FullCoverInstancedMeshComp->SetFlags(RF_Transactional);
	this->AddInstanceComponent(FullCoverInstancedMeshComp);

	HalfCoverInstancedMeshComp = CreateDefaultSubobject<UInstancedStaticMeshComponent>(FName("HalfCoverInstancedMesh"));
	HalfCoverInstancedMeshComp->RegisterComponent();
	HalfCoverInstancedMeshComp->SetFlags(RF_Transactional);
	this->AddInstanceComponent(HalfCoverInstancedMeshComp);

}

void ACoveringChecker::BeginPlay()
{
	Super::BeginPlay();
	if (CoverMesh && FullCoverMaterial && HalfCoverMaterial)
	{
		FullCoverMaterialDynamic = UMaterialInstanceDynamic::Create(FullCoverMaterial, this);
		HalfCoverMaterialDynamic = UMaterialInstanceDynamic::Create(HalfCoverMaterial, this);

		FString ImagePath = "/Game/Material/Shield_border_mat_2.Shield_border_mat_2";  // �������� ���� (���Ͽ� ����)
		UMaterial* BorderMaterial = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), NULL, *(ImagePath)));

		FullCoverInstancedMeshComp->SetStaticMesh(CoverMesh);
		FullCoverInstancedMeshComp->SetMaterial(0, FullCoverMaterialDynamic);
		FullCoverInstancedMeshComp->SetMaterial(1, BorderMaterial);

		HalfCoverInstancedMeshComp->SetStaticMesh(CoverMesh);
		HalfCoverInstancedMeshComp->SetMaterial(0, HalfCoverMaterialDynamic);
		HalfCoverInstancedMeshComp->SetMaterial(1, HalfCoverMaterialDynamic);
	}
	else 
	{
		UE_LOG(LogTemp, Warning, L"�޽� ��� �ʿ�")
	}
}

void ACoveringChecker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

/**
* ���󰡴��� ���� ���� Mesh�� �����մϴ�.
* @param TileLocationArray - Cover Mesh������ �ĺ��� �� Ÿ�� ��ġ �迭
* @param Spacing - �޽� ������ ���� ����
*/
void ACoveringChecker::MakingCoverNotice(TArray<FVector>& TileLocationArray, float Spacing) 
{
	if (!CoverMesh || !FullCoverMaterial || !HalfCoverMaterial) { return; }
	TArray<FTransform> FullCoverNoticeTF;
	TArray<FTransform> HalfCoverNoticeTF;

	for (FVector TileLocation : TileLocationArray) 
	{
		RayCastToCardinalDirection(TileLocation, Spacing, HalfCoverNoticeTF, FullCoverNoticeTF);
	}

	AddFullCoverInstances(FullCoverNoticeTF);
	AddHalfCoverInstances(HalfCoverNoticeTF);
}

/**
* RayCast�� ����⿡ ������ Mesh���� ���� ��ġ�� �����ϴ�.
* @param OriginLocation - Ÿ���� ��ġ
* @param Spacing - �޽� ������ ���� ����
* @param HalfCoverNoticeTF - �� ���� �޽��� ������ ��ġ �迭
* @param FullCoverNoticeTF - ���� ���� �޽��� ������ ��ġ �迭
*/
void ACoveringChecker::RayCastToCardinalDirection(FVector OriginLocation, float Spacing, OUT TArray<FTransform>& HalfCoverNoticeTF, OUT TArray<FTransform>& FullCoverNoticeTF)
{
	FVector CardinalLocation[4] = { 
		OriginLocation + FVector(Spacing, 0, 0),
		OriginLocation + FVector(-Spacing, 0, 0),
		OriginLocation + FVector(0, Spacing, 0),
		OriginLocation + FVector(0, -Spacing, 0)
		};

	FCollisionQueryParams CollisionParams(FName(TEXT("FOW trace")), false);
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
			FVector ShieldLocation = HitResult.ImpactPoint + ((HitResult.ImpactNormal) * 10) + FVector(0, 0, 50);
			ShieldTransform.SetLocation(ShieldLocation);
			ShieldTransform.SetRotation((-HitResult.ImpactNormal).ToOrientationQuat());
			ShieldTransform.SetScale3D(FVector(1, 1, 1));

			ADestructibleWall* DestructibleWall = Cast<ADestructibleWall>(HitResult.GetActor());
			if (DestructibleWall) 
			{
				if (DestructibleWall->CoverInfo == ECoverInfo::FullCover) 
				{
					FullCoverNoticeTF.Add(ShieldTransform);
				}
				else 
				{
					HalfCoverNoticeTF.Add(ShieldTransform);
				}
			}
		}
	}
}


void ACoveringChecker::AddFullCoverInstances(TArray<FTransform>& TransformArray) const
{
	for (FTransform SingleTransform : TransformArray)
	{
		FullCoverInstancedMeshComp->AddInstanceWorldSpace(SingleTransform);
	}
}

void ACoveringChecker::AddHalfCoverInstances(TArray<FTransform>& TransformArray) const
{
	for (FTransform SingleTransform : TransformArray)
	{
		HalfCoverInstancedMeshComp->AddInstanceWorldSpace(SingleTransform);
	}
}

void ACoveringChecker::ClearAllCoverNotice()
{
	HalfCoverInstancedMeshComp->ClearInstances();
	FullCoverInstancedMeshComp->ClearInstances();
}