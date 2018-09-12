// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Unit/CustomThirdPerson.h"
#include "EnemyUnit.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FPlayAggroEventDelegate, AActor*, PlayUnit);
DECLARE_DYNAMIC_DELEGATE_OneParam(FRegisterEventDelegate, AActor*, PlayUnit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFinishAggroEventDelegate);
/**
 * 
 */
UCLASS()
class XCOM_API AEnemyUnit : public ACustomThirdPerson
{
	GENERATED_BODY()
public:
	AEnemyUnit();

	FPlayAggroEventDelegate PlayAggroEventDelegate;

	FFinishAggroEventDelegate FinishAggroEventDelegate;

	FRegisterEventDelegate RegisterEventDelegate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UAnimMontage* EmoteMontage;

	/** �׷� ��ȣ */
	UPROPERTY(VisibleAnywhere)
	int8 Group;

	int8 GetGroupNumber() { return Group; }

	bool IsAggro() { return bAggro; };

	void OnAggo() { bAggro = true; };

	/** ������ ���ӿ��� ����ϴ�. */
	void HideUnit();

	/** ������ ���ӿ� ��Ÿ���� �մϴ�.*/
	void UnHideUnit();

	/** ������ ��(����)�� �ѱ�ϴ�. */
	void ForceOverTurn();

	/** ����ǥ�� ���� �� ȣ��˴ϴ�. */
	UFUNCTION()
	void OnEmoteMontageEnded();

	void PlayEvent();

protected:

	/** �������� �����մϴ�.*/
	virtual void FinishMoving() override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser) override;

	virtual void Dead() override;

private:
	/** Aggro ���� */
	bool bAggro = false;

	/** �÷��̾��� ������ �߰������� ����ǥ���� �մϴ�.*/
	void PlayEmoteMontage();


};
