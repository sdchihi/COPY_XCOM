// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyUnit.h"
#include "Kismet/KismetSystemLibrary.h"
#include "CustomThirdPerson.h"
#include "XCOMGameMode.h"
#include "Classes/Animation/AnimMontage.h"
#include "Public/UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "EnemyController.h"

AEnemyUnit::AEnemyUnit() 
{
	WalkingState = EWalkingState::Walk;
}

/**
* 움직임을 종료합니다.
*/
void AEnemyUnit::FinishMoving() 
{
	bool bChangeAggro = false;
	TArray<AActor*> OutActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> Fillter;
	if (!bAggro) 
	{
		bool bFliteredUnit = UKismetSystemLibrary::SphereOverlapActors
		(
			GetWorld(),
			GetActorLocation(),
			AttackRadius,
			Fillter,
			ACustomThirdPerson::StaticClass(),
			TArray<AActor*>(),
			OutActors
		);

		//거리 안에 적이있어서 확인될경우 어그로 변경 tODO 팀 확인
		AXCOMGameMode* GameMode = Cast<AXCOMGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

		if (bFliteredUnit) //적이 시야 안에있을때
		{
			GameMode->ChangeEnemyAggro(Group);
			bChangeAggro = true;
		}
		else // 시야안에 없지만 다른 그룹이 전투 상태에 있고,UnFogArea 안에 위치했을때 
		{
			if (GameMode->GetEnemysBattleCognition() == true && IsInUnFoggedArea())
			{
				GameMode->ChangeEnemyAggro(Group);
				bChangeAggro = true;
			}
		}
	}

	if (bChangeAggro) 
	{
		if (RegisterEventDelegate.IsBound())
		{
			RegisterEventDelegate.Execute(this);
		}
	}
	Super::FinishMoving();
}


float AEnemyUnit::TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser) 
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	UE_LOG(LogTemp, Warning, L"^데미지 받긴함")

	AXCOMGameMode* GameMode = Cast<AXCOMGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode->GetEnemysBattleCognition() == false) 
	{
		GameMode->SetEnemysBattleCognition();
	}

	if (bAggro == false) 
	{
		GameMode->ChangeEnemyAggro(Group);
		if (CurrentHP > 0) 
		{
			UE_LOG(LogTemp, Warning, L"^등록신청함")

			if (RegisterEventDelegate.IsBound())
			{
				RegisterEventDelegate.Execute(this);
			}
		}
		else 
		{
			UE_LOG(LogTemp, Warning, L"^HP 바닥났대")
		}
	}
	return ActualDamage;
}

/**
* 유닛을 게임에서 숨깁니다.
*/
void AEnemyUnit::HideUnit()
{
	SetActorHiddenInGame(true);
	if (GunReference) 
	{
		GunReference->SetActorHiddenInGame(true);
	}
	SetHealthBarVisibility(false);
};

/**
* 유닛을 게임에 나타나게 합니다.
*/
void AEnemyUnit::UnHideUnit()
{
	SetActorHiddenInGame(false);
	if (GunReference)
	{
		GunReference->SetActorHiddenInGame(false);
	}
	SetHealthBarVisibility(true);
}

/**
* 플레이어의 유닛을 발견했을때 감정표현을 합니다.
*/
void AEnemyUnit::PlayEmoteMontage() 
{
	if (EmoteMontage) 
	{
		float FuncCallDelay = PlayAnimMontage(EmoteMontage);

		if (PlayAggroEventDelegate.IsBound()) 
		{
			PlayAggroEventDelegate.Execute(this);
		}
		FTimerHandle UnUsedHandle;
		FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &AEnemyUnit::OnEmoteMontageEnded);
		GetWorldTimerManager().SetTimer(UnUsedHandle, TimerDelegate, FuncCallDelay, false);
	}
}

/**
* 감정표현 종료 후 호출됩니다.
*/
void AEnemyUnit::OnEmoteMontageEnded() 
{
	if (FinishAggroEventDelegate.IsBound()) 
	{
		FinishAggroEventDelegate.Broadcast();
	}
}

/**
* 강제로 턴(제어)를 넘깁니다.
*/
void AEnemyUnit::ForceOverTurn() 
{
	bCanAction = false;
	if (AfterActionDelegate.IsBound()) 
	{
		AfterActionDelegate.Broadcast(GetTeamFlag());
	}
}

void AEnemyUnit::PlayEvent() 
{
	PlayEmoteMontage();
}

void AEnemyUnit::Dead() 
{
	Super::Dead();
	
	AEnemyController* EnemyController = Cast<AEnemyController>(GetController());
	EnemyController->StopBehaviorTree();
}
