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
	/** FAimingQueue �̱��� ��ü*/
	static FAimingQueue* pInstance;

	/** Queue�� Head�� ����Ű�� �ε��� ��*/
	static int32 Head;

	/** Queue�� Tail�� ����Ű�� �ε��� ��*/
	static int32 Tail;

	/** Queue�� ũ��*/
	static const int32 MaxPending = 10;

	/** �迭�� ������ ť */
	static FOrderlyAimingInfo* Pending[MaxPending];

	/** ���������� ������� Actor�� ������ */
	static ACustomThirdPerson* LastShootingActor;

	static FOrderlyAimingInfo* GetPending() { return *Pending; };

	/**
	* AimingQueue�� ��ü�� ���ɴϴ�. (�̱��� ��ü)
	* @return AimingQueue pointer
	*/
	static FAimingQueue& Instance();

	/** Queue���� ��ȿ�� AimingInfo�� ��� ����� �����մϴ�.*/
	static void Update();

	/** ���� �۾����� �����մϴ�. */
	static void NextTask();

	static void Destroy();

	/**
	* Queue�� Aiminig Info�� �߰��մϴ�.
	* @param AimingActor - ����� ��ü�� �Ǵ� Actor
	* @param AimingInfo - ��ݿ� ���õ� ����
	*/
	static void StartAiming(ACustomThirdPerson* AimingActor, FAimingInfo* AimingInfo);

private:	
	FAimingQueue();

	/**
	* Head�� Tail�� Index���̰� 1 ���� Ȯ���մϴ�.
	* @return Head, Tail�� ���̰� 1���� ����
	*/
	static bool IsPrevTask();
	
};
