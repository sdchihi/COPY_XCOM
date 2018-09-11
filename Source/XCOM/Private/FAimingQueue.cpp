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

/**
* AimingQueue의 객체를 얻어옵니다. (싱글턴 객체)
* @return AimingQueue pointer
*/
FAimingQueue& FAimingQueue::Instance() 
{
	if (pInstance == nullptr) 
	{
		FAimingQueue::pInstance = new FAimingQueue();
	}

	return *pInstance;
}

/**
* Queue에서 유효한 AimingInfo의 경우 사격을 지시합니다.
*/
void FAimingQueue::Update() 
{
	ACustomThirdPerson* AimingCharacter = FAimingQueue::Pending[FAimingQueue::Head]->AimingActor;
	FAimingInfo* CurrentAimingInfo = FAimingQueue::Pending[FAimingQueue::Head]->AimingInfo;
	ACustomThirdPerson* AimedCharcater = Cast<ACustomThirdPerson>(CurrentAimingInfo->TargetActor);
	UE_LOG(LogTemp, Warning, L"관찰  Update  Head - %d      Tail - %d ", FAimingQueue::Head, FAimingQueue::Tail)
	UE_LOG(LogTemp, Warning, L"관찰  Update2  주체 - %s       타겟 - %s ", *AimingCharacter->GetName(), *AimedCharcater->GetName())

	if (!IsValid(AimedCharcater))
	{
		return;
	}
	AEnemyController* EnemyController = Cast<AEnemyController>(AimedCharcater->GetController());
	AimedCharcater->CustomTimeDilation = 0;

	if (AimingCharacter->bInVisilance) 
	{
		LastShootingActor = AimingCharacter;
		AimingCharacter->InformVisilanceSuccess(AimingCharacter->GetActorLocation(), CurrentAimingInfo->TargetLocation);
		AimingCharacter->AttackEnemyAccrodingToState(*CurrentAimingInfo);
		AimingCharacter->StopVisilance();
	}
	else
	{
		FAimingQueue::NextTask();
	}
}

/**
* Queue에 Aiminig Info를 추가합니다.
* @param AimingActor - 사격의 주체가 되는 Actor
* @param AimingInfo - 사격에 관련된 정보
*/
void FAimingQueue::StartAiming(ACustomThirdPerson* AimingActor, FAimingInfo* AimingInfo)
{
	int32 Temp_Head = FAimingQueue::Head;
	int32 Temp_Tail = FAimingQueue::Tail;
	FAimingQueue::Pending[Temp_Tail] =new FOrderlyAimingInfo(AimingActor, AimingInfo);

	FAimingQueue::Tail = (FAimingQueue::Tail + 1) % MaxPending;
	if (Temp_Head == Temp_Tail)
	{
		FAimingQueue::Update();

		UE_LOG(LogTemp, Warning, L"관찰  StartAiming Head - %d      Tail - %d ", FAimingQueue::Head, FAimingQueue::Tail)
	}
}

/**
* 다음 작업으로 진행합니다.
*/
void FAimingQueue::NextTask() 
{
	ACustomThirdPerson* AimedCharcater = Cast<ACustomThirdPerson>(FAimingQueue::Pending[FAimingQueue::Head]->AimingInfo->TargetActor);

	if (IsPrevTask()) // Finish Task
	{
		LastShootingActor->ExecuteChangePawnDelegate();
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

/**
* Head와 Tail의 Index차이가 1 인지 확인합니다.
* @return Head, Tail의 차이가 1인지 여부
*/
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
