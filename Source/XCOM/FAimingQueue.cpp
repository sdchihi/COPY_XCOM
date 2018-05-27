// Fill out your copyright notice in the Description page of Project Settings.

#include "FAimingQueue.h"
#include "CustomThirdPerson.h"
#include "AimingComponent.h"
#include "Engine.h"

FAimingQueue* FAimingQueue::pInstance;
int32 FAimingQueue::Head;
int32 FAimingQueue::Tail;
int32 FAimingQueue::Temp;
FOrderlyAimingInfo* FAimingQueue::Pending[MaxPending];


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
		UE_LOG(LogTemp, Warning, L"VV �߸��� ����")

		return;
	}
	else 
	{
		UE_LOG(LogTemp, Warning, L"VV ��ȿ���� : %s" ,*AimedCharcater->GetName())

	}
	AimedCharcater->CustomTimeDilation = 0;
	
	UE_LOG(LogTemp, Warning, L"VV �˻� : %f", CurrentAimingInfo->Probability);

	if (CurrentAimingInfo->Probability == 0)
	{
		UE_LOG(LogTemp, Warning, L"VV �߸��� ����2")

		return;
	}
	else if (CurrentAimingInfo->Factor.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, L"VV �߸��� ����3")

		return;
	}

	AimingCharacter->InformVisilanceSuccess(AimingCharacter->GetActorLocation(), CurrentAimingInfo->TargetLocation);
	AimingCharacter->AttackEnemyAccrodingToState(*CurrentAimingInfo);
	
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
		UE_LOG(LogTemp, Warning, L"VV 1�� �б�")

	}
	else 
	{
		UE_LOG(LogTemp, Warning, L"VV 2�� �б�")
		FAimingQueue::Tail = (FAimingQueue::Tail + 1) % MaxPending;
	}
	//UE_LOG(LogTemp, Warning, L"VV Head : %d Tail : %d", FAimingQueue::Head, FAimingQueue::Tail)

}

void FAimingQueue::NextTask() 
{
	UE_LOG(LogTemp, Warning, L"VV In NextTask")

	if (IsPrevTask()) // Finish Task
	{
		FAimingQueue::Pending[FAimingQueue::Head]->AimingActor->SetOffAttackState(true);
		UE_LOG(LogTemp, Warning, L"VV Finished")
		FAimingQueue::Head = (FAimingQueue::Head + 1) % MaxPending;
	}
	else 
	{
		UE_LOG(LogTemp, Warning, L"VV Continue")
		
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