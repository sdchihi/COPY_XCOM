// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Unit/CustomThirdPerson.h"
#include "EnemyUnit.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FPlayAggroEventDelegate, AActor*, PlayUnit);
DECLARE_DYNAMIC_DELEGATE(FFinishAggroEventDelegate);

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

	FFinishAggroEventDelegate FinishAggroEventDelegate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UAnimMontage* EmoteMontage;


	UFUNCTION()
	void OnEmoteMontageEnded();

	//void OnEmoteMontageEnded(UAnimMontage* Montage, bool bInterrupted);

protected:
	virtual void FinishMoving() override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser) override;


private:
	UPROPERTY(EditDefaultsOnly)
	int8 Group;
	
	void PlayEmoteMontage();

	void FinishMovingAfterMontage();


	bool bAggro = false;
};
