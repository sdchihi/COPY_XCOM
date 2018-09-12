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

	/** 그룹 번호 */
	UPROPERTY(VisibleAnywhere)
	int8 Group;

	int8 GetGroupNumber() { return Group; }

	bool IsAggro() { return bAggro; };

	void OnAggo() { bAggro = true; };

	/** 유닛을 게임에서 숨깁니다. */
	void HideUnit();

	/** 유닛을 게임에 나타나게 합니다.*/
	void UnHideUnit();

	/** 강제로 턴(제어)를 넘깁니다. */
	void ForceOverTurn();

	/** 감정표현 종료 후 호출됩니다. */
	UFUNCTION()
	void OnEmoteMontageEnded();

	void PlayEvent();

protected:

	/** 움직임을 종료합니다.*/
	virtual void FinishMoving() override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser) override;

	virtual void Dead() override;

private:
	/** Aggro 여부 */
	bool bAggro = false;

	/** 플레이어의 유닛을 발견했을때 감정표현을 합니다.*/
	void PlayEmoteMontage();


};
