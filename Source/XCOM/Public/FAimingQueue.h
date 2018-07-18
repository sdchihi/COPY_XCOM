// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class ACustomThirdPerson;
struct FAimingInfo;

struct FOrderlyAimingInfo
{
	ACustomThirdPerson* AimingActor;

	FAimingInfo* AimingInfo;

	FOrderlyAimingInfo(ACustomThirdPerson* AimingActor_, FAimingInfo* AimingInfo_)
	{
		AimingActor = AimingActor_; 
		AimingInfo = AimingInfo_;
	}

	~FOrderlyAimingInfo(ACustomThirdPerson* AimingActor_, FAimingInfo* AimingInfo_)
	{
		delete AimingInfo;
	}

};

/**
 * 
 */
class XCOM_API FAimingQueue
{
public:
	static FAimingQueue& Instance();

	static void Update();

	static void NextTask();

	static void Destroy();

	static void StartAiming(ACustomThirdPerson* AimingActor, FAimingInfo* AimingInfo);

	static FAimingQueue* pInstance;

	static int32 Head;

	static int32 Tail;

	static const int32 MaxPending = 10;

	static FOrderlyAimingInfo* Pending[MaxPending];

	bool bStart;

	static FOrderlyAimingInfo* GetPending() { return *Pending; };

	static ACustomThirdPerson* LastShootingActor;

private:	


	FAimingQueue();
	~FAimingQueue();
	static bool IsPrevTask();
	
};
