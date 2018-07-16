// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyUnit.h"
#include "Kismet/KismetSystemLibrary.h"
#include "CustomThirdPerson.h"
#include "XCOMGameMode.h"
#include "Classes/Animation/AnimMontage.h"
#include "Public/UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Public/TimerManager.h"

AEnemyUnit::AEnemyUnit() 
{
	WalkingState = EWalkingState::Walk;
}

void AEnemyUnit::FinishMoving() 
{
	bool bChangeAggro = false;
	//여기 밑으론 일단 매번하면 안되요요오
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
			UE_LOG(LogTemp,Warning,L"발견!")
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
	//여기까지 아직 Point 사용전. 즉 Cut scene event 처리하기 위한 위치 

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
	// 이부분 수정- > 맞고나서 다 끝난 후 이벤트 처리 필요
	return ActualDamage;
}

void AEnemyUnit::HideUnit()
{
	SetActorHiddenInGame(true);
	GunReference->SetActorHiddenInGame(true);
	HealthBar->SetVisibilityLocker(true);
	HealthBar->SetWidgetVisibility(true);
};


void AEnemyUnit::UnHideUnit()
{
	SetActorHiddenInGame(false);
	GunReference->SetActorHiddenInGame(false);
	HealthBar->SetVisibilityLocker(false);
	HealthBar->SetWidgetVisibility(true);
}

void AEnemyUnit::PlayEmoteMontage() 
{
	if (EmoteMontage) 
	{
		//Montage_SetEndDelegate가 작동안해서 그냥 FTimerDelegate로 Montage_SetEndDelegate 대체

		//FOnMontageEnded EndDelegate;
		//EndDelegate.BindUObject(this, &AEnemyUnit::OnEmoteMontageEnded);
		////EmoteMontage->Montage_SetEndDelegate(EndDelegate);
		//GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(EndDelegate);
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

void AEnemyUnit::OnEmoteMontageEnded() 
{
	if (FinishAggroEventDelegate.IsBound()) 
	{
		FinishAggroEventDelegate.Broadcast();
	}
}


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

void AEnemyUnit::FinishEvent() 
{

}
