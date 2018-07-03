// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Unit/CustomThirdPerson.h"
#include "EnemyUnit.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FPlayAggroEventDelegate, AEnemyUnit*, PlayUnit);

/**
 * 
 */
UCLASS()
class XCOM_API AEnemyUnit : public ACustomThirdPerson
{
	GENERATED_BODY()
public:
	AEnemyUnit();

	int8 GetGroupNumber() { return Group; }

	bool IsAggro() { return bAggro; };

	void OnAggo() { bAggro = true; };

	void HideUnit();

	void UnHideUnit();

	FPlayAggroEventDelegate PlayAggroEventDelegate;

protected:
	virtual void FinishMoving() override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser) override;


private:
	UPROPERTY(EditDefaultsOnly)
	int8 Group;
	

	bool bAggro = false;
};
