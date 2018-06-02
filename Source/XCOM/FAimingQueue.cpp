// Fill out your copyright notice in the Description page of Project Settings.

#include "FAimingQueue.h"
#include "CustomThirdPerson.h"
#include "AimingComponent.h"
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
	AActor* AimedCharcater = CurrentAimingInfo->TargetActor;


	if (!IsValid(AimedCharcater))
	{
		return;
	}
	AimedCharcater->CustomTimeDilation = 0;
	
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
		LastShootingActor->SetOffAttackState(true);	//�����ʿ��ؿ� - > ��ݾ��ߴµ��� SetOff�� �� �ֳ�.
		AActor* AimedCharcater = FAimingQueue::Pending[FAimingQueue::Head]->AimingInfo->TargetActor;
		if (IsValid(AimedCharcater)) 
		{
			AimedCharcater->CustomTimeDilation = 1;
		}
		FAimingQueue::Head = (FAimingQueue::Head + 1) % MaxPending;
	}
	else 
	{
		FAimingQueue::Head = (FAimingQueue::Head + 1) % MaxPending;
		ACustomThirdPerson* tempee=   FAimingQueue::Pending[Head]->AimingActor;
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