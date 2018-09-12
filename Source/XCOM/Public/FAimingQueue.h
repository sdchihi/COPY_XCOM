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

	~FOrderlyAimingInfo()
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
	/** FAimingQueue 싱글턴 객체*/
	static FAimingQueue* pInstance;

	/** Queue의 Head를 가르키는 인덱스 값*/
	static int32 Head;

	/** Queue의 Tail를 가르키는 인덱스 값*/
	static int32 Tail;

	/** Queue의 크기*/
	static const int32 MaxPending = 10;

	/** 배열로 구현된 큐 */
	static FOrderlyAimingInfo* Pending[MaxPending];

	/** 마지막으로 사격을한 Actor의 포인터 */
	static ACustomThirdPerson* LastShootingActor;

	static FOrderlyAimingInfo* GetPending() { return *Pending; };

	/**
	* AimingQueue의 객체를 얻어옵니다. (싱글턴 객체)
	* @return AimingQueue pointer
	*/
	static FAimingQueue& Instance();

	/** Queue에서 유효한 AimingInfo의 경우 사격을 지시합니다.*/
	static void Update();

	/** 다음 작업으로 진행합니다. */
	static void NextTask();

	static void Destroy();

	/**
	* Queue에 Aiminig Info를 추가합니다.
	* @param AimingActor - 사격의 주체가 되는 Actor
	* @param AimingInfo - 사격에 관련된 정보
	*/
	static void StartAiming(ACustomThirdPerson* AimingActor, FAimingInfo* AimingInfo);

private:	
	FAimingQueue();

	/**
	* Head와 Tail의 Index차이가 1 인지 확인합니다.
	* @return Head, Tail의 차이가 1인지 여부
	*/
	static bool IsPrevTask();
	
};
