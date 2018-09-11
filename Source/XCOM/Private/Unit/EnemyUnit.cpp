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
* �������� �����մϴ�.
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

		//�Ÿ� �ȿ� �����־ Ȯ�εɰ�� ��׷� ���� tODO �� Ȯ��
		AXCOMGameMode* GameMode = Cast<AXCOMGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

		if (bFliteredUnit) //���� �þ� �ȿ�������
		{
			GameMode->ChangeEnemyAggro(Group);
			bChangeAggro = true;
		}
		else // �þ߾ȿ� ������ �ٸ� �׷��� ���� ���¿� �ְ�,UnFogArea �ȿ� ��ġ������ 
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
	UE_LOG(LogTemp, Warning, L"^������ �ޱ���")

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
			UE_LOG(LogTemp, Warning, L"^��Ͻ�û��")

			if (RegisterEventDelegate.IsBound())
			{
				RegisterEventDelegate.Execute(this);
			}
		}
		else 
		{
			UE_LOG(LogTemp, Warning, L"^HP �ٴڳ���")
		}
	}
	return ActualDamage;
}

/**
* ������ ���ӿ��� ����ϴ�.
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
* ������ ���ӿ� ��Ÿ���� �մϴ�.
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
* �÷��̾��� ������ �߰������� ����ǥ���� �մϴ�.
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
* ����ǥ�� ���� �� ȣ��˴ϴ�.
*/
void AEnemyUnit::OnEmoteMontageEnded() 
{
	if (FinishAggroEventDelegate.IsBound()) 
	{
		FinishAggroEventDelegate.Broadcast();
	}
}

/**
* ������ ��(����)�� �ѱ�ϴ�.
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
