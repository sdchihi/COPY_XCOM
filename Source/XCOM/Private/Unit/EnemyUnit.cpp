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
	//���� ������ �ϴ� �Ź��ϸ� �ȵǿ���
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

		if (bFliteredUnit)
		{
			UE_LOG(LogTemp,Warning,L"�߰�!")
			GameMode->ChangeEnemyAggro(Group);
			bChangeAggro = true;
		}
		else 
		{
			if (GameMode->GetEnemysBattleCognition() == true && IsInUnFoggedArea())
			{
				GameMode->ChangeEnemyAggro(Group);
				bChangeAggro = true;
			}
		}
	}
	//������� ���� Point �����. �� Cut scene event ó���ϱ� ���� ��ġ 

	if (bChangeAggro) 
	{
		FinishMovingAfterMontage();
	}
	else 
	{
		Super::FinishMoving();
	}


}

void AEnemyUnit::FinishMovingAfterMontage() 
{
	PlayEmoteMontage();
}

	

float AEnemyUnit::TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser) 
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	AXCOMGameMode* GameMode = Cast<AXCOMGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode->GetEnemysBattleCognition() == false) 
	{
		GameMode->SetEnemysBattleCognition();
	}

	if (bAggro == false) 
	{
		GameMode->ChangeEnemyAggro(Group);
		if (PlayAggroEventDelegate.IsBound())
		{
			PlayAggroEventDelegate.Execute(this);
		}
	}

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
		//Montage_SetEndDelegate�� �۵����ؼ� �׳� FTimerDelegate�� Montage_SetEndDelegate ��ü

		//FOnMontageEnded EndDelegate;
		//EndDelegate.BindUObject(this, &AEnemyUnit::OnEmoteMontageEnded);
		////EmoteMontage->Montage_SetEndDelegate(EndDelegate);
		//GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(EndDelegate);
		float FuncCallDelay = PlayAnimMontage(EmoteMontage);

		if (PlayAggroEventDelegate.IsBound()) 
		{
			PlayAggroEventDelegate.Execute(Cast<AActor>(this));
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
		FinishAggroEventDelegate.Execute();
	}
	Super::FinishMoving();
}
