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

//FAimingQueue::~FAimingQueue()
//{
//	/*for (auto PendingToKill : FAimingQueue::Pending)
//	{
//		if (PendingToKill != nullptr && sizeof(PendingToKill) == sizeof(FOrderlyAimingInfo))
//		{
//			delete PendingToKill;
//		}
//	}*/
//}


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
	UE_LOG(LogTemp, Warning, L"����  Update  Head - %d      Tail - %d ", FAimingQueue::Head, FAimingQueue::Tail)
	UE_LOG(LogTemp, Warning, L"����  Update2  ��ü - %s       Ÿ�� - %s ", *AimingCharacter->GetName(), *AimedCharcater->GetName())

	if (!IsValid(AimedCharcater))
	{
		UE_LOG(LogTemp, Warning, L"����3  ����")

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
	else //ť�� �������� �̹� ����� ��� -> ���� �ε����� �Ѿ���� 
	{
		FAimingQueue::NextTask();
	}
}


void FAimingQueue::StartAiming(ACustomThirdPerson* AimingActor, FAimingInfo* AimingInfo)
{
	int32 Temp_Head = FAimingQueue::Head;
	int32 Temp_Tail = FAimingQueue::Tail;
	FAimingQueue::Pending[Temp_Tail] =new FOrderlyAimingInfo(AimingActor, AimingInfo);

	FAimingQueue::Tail = (FAimingQueue::Tail + 1) % MaxPending;
	if (Temp_Head == Temp_Tail)
	{
		FAimingQueue::Update();

		UE_LOG(LogTemp, Warning, L"����  StartAiming Head - %d      Tail - %d ", FAimingQueue::Head, FAimingQueue::Tail)
	}
}

void FAimingQueue::NextTask() 
{
	ACustomThirdPerson* AimedCharcater = Cast<ACustomThirdPerson>(FAimingQueue::Pending[FAimingQueue::Head]->AimingInfo->TargetActor);

	if (IsPrevTask()) // Finish Task
	{
		LastShootingActor->ExecuteChangePawnDelegate();//
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
		if (AimedCharcater->IsDead())
		{
			NextTask();
		}
		else 
		{
			FAimingQueue::Update();
		}
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
	if (pInstance != nullptr) 
	{
		delete pInstance;
	}
};
