// Fill out your copyright notice in the Description page of Project Settings.

#include "FAimingQueue.h"
#include "CustomThirdPerson.h"
#include "AimingComponent.h"
#include "EnemyController.h"
#include "Engine.h"


FAimingQueue* FAimingQueue::pInstance;
int32 FAimingQueue::Head;
int32 FAimingQueue::Tail;
FOrderlyAimingInfo* FAimingQueue::Pending[MaxPending];
ACustomThirdPerson* FAimingQueue::LastShootingActor;


FAimingQueue::FAimingQueue()
{
	Head = 0;
	Tail = 0;
}

FAimingQueue::~FAimingQueue()
{
	for (FOrderlyAimingInfo* PendingToKill : Pending) 
	{
		if (PendingToKill != nullptr) 
		{
			delete PendingToKill;
		}
	}
}


FAimingQueue& FAimingQueue::Instance() 
{
	if (pInstance == nullptr) 
	{
		FAimingQueue::pInstance = new FAimingQueue();
	}

	return *pInstance;
}


void FAimingQueue::Update() 
{
	ACustomThirdPerson* AimingCharacter = FAimingQueue::Pending[FAimingQueue::Head]->AimingActor;
	FAimingInfo* CurrentAimingInfo = FAimingQueue::Pending[FAimingQueue::Head]->AimingInfo;
	ACustomThirdPerson* AimedCharcater = Cast<ACustomThirdPerson>(CurrentAimingInfo->TargetActor);
	if (!IsValid(AimedCharcater))
	{
		return;
	}
	AEnemyController* EnemyController = Cast<AEnemyController>(AimedCharcater->GetController());

	AimedCharcater->CustomTimeDilation = 0;
	if (EnemyController) 
	{
		//EnemyController->StopBehaviorTree();
	}

	if (AimingCharacter->bInVisilance) 
	{
		LastShootingActor = AimingCharacter;
		AimingCharacter->InformVisilanceSuccess(AimingCharacter->GetActorLocation(), CurrentAimingInfo->TargetLocation);
		AimingCharacter->AttackEnemyAccrodingToState(*CurrentAimingInfo);
		AimingCharacter->StopVisilance();
	}
	else //큐에 들어왔지만 이미 사격한 경우 -> 다음 인덱스로 넘어가야함 
	{
		FAimingQueue::NextTask();
	}
}


void FAimingQueue::StartAiming(ACustomThirdPerson* AimingActor, FAimingInfo* AimingInfo)
{
	int32 Temp_Head = FAimingQueue::Head;
	int32 Temp_Tail = FAimingQueue::Tail;
	FAimingQueue::Pending[Temp_Tail] =new FOrderlyAimingInfo(AimingActor, AimingInfo);

	if (FAimingQueue::Head == FAimingQueue::Tail)
	{
		FAimingQueue::Tail = (FAimingQueue::Tail + 1) % MaxPending;
		FAimingQueue::Update();
	}
	else 
	{
		FAimingQueue::Tail = (FAimingQueue::Tail + 1) % MaxPending;
	}
}

void FAimingQueue::NextTask() 
{
	if (IsPrevTask()) // Finish Task
	{
		LastShootingActor->ExecuteChangePawnDelegate();//
		ACustomThirdPerson* AimedCharcater = Cast<ACustomThirdPerson>(FAimingQueue::Pending[FAimingQueue::Head]->AimingInfo->TargetActor);
		if (IsValid(AimedCharcater)) 
		{
			AimedCharcater->CustomTimeDilation = 1;
			AEnemyController* EnemyController = Cast<AEnemyController>(AimedCharcater->GetInstigatorController());
			if (EnemyController) 
			{
				//EnemyController->StartBehaviorTree();
			}
		}
		FAimingQueue::Head = (FAimingQueue::Head + 1) % MaxPending;
	}
	else 
	{
		FAimingQueue::Head = (FAimingQueue::Head + 1) % MaxPending;
		FAimingQueue::Update();
	}
}


bool FAimingQueue::IsPrevTask() 
{
	int32 MaximumIndex = MaxPending - 1;
	if (FAimingQueue::Head == (FAimingQueue::Tail - 1)) 
	{
		return true;
	}
	else if (FAimingQueue::Tail == 0 && FAimingQueue::Head == MaximumIndex)
	{
		return true;
	}
	else 
	{
		return false;
	}
}

void FAimingQueue::Destroy() 
{
	delete pInstance;
};
