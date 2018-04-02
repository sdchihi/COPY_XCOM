// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DestructibleWall.h"
#include "AimingComponent.h"
#include "CustomThirdPerson.generated.h"

class AGun;
class USpringArmComponent;
class UCameraComponent;
class UAimingComponent;

DECLARE_DYNAMIC_DELEGATE(FChangePlayerPawnDelegate);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCheckTurnDelegate , bool , bIsPlayerTeam);

UENUM(BlueprintType)
enum class ECoverDirection : uint8
{
	South,
	North,
	East,
	West,
	None
};


UCLASS()
class XCOM_API ACustomThirdPerson : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACustomThirdPerson();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	ECoverDirection CoverDirection = ECoverDirection::None;
	
	TMap<ECoverDirection, ECoverInfo> CoverDirectionMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AGun> GunBlueprint;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	AGun* GunReference;

	//void CheckAttackPotential(APawn* TargetPawn);

	bool GetTeamFlag() { return bTeam; };

	void ClearCoverDirectionInfo();

	UPROPERTY(BlueprintReadOnly)
	bool bIsCovering = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsAttack = false;

	bool bCanAction = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int MaxHP = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int CurrentHP;

	UPROPERTY(EditDefaultsOnly)
	float AttackRadius = 1000;

	int32 CurrentMovableStep;
	
	int32 GetMovableStepPerActionPoint() { return Step / 2; };

	void RotateTowardWall();

	FChangePlayerPawnDelegate ChangePlayerPawnDelegate;

	FCheckTurnDelegate CheckTurnDelegate;

	UPROPERTY(EditDefaultsOnly)
	int32 RemainingActionPoint = 2;

	UFUNCTION(BlueprintImplementableEvent)
	void AimAt(FVector AimDirection, FName NewCollisionPresetName);

	UFUNCTION(BlueprintCallable)
	void RotateCharacter(FVector AimDirection, float LerpAlpha);

	UFUNCTION(BlueprintCallable)
	void StartFiring(FName NewCollisionPresetName);

	int32 GetStep() { return Step; };

	void UseActionPoint(int32 PointToUse);

	void RestoreActionPoint();

	void ScanEnemy();

	TArray<FAimingInfo>& GetAimingInfo() { return AimingInfo; };

protected:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser) override;

private:
	UAimingComponent* AimingComponent = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	bool bTeam = true;

	UPROPERTY(EditDefaultsOnly)
	int32 Step = 10;

	void SetOffAttackState();

	FVector RelativeCoverLoc = FVector(0,0,0);

	TArray<FAimingInfo> AimingInfo;

	//이후에 클래스로 묶일 능력치들
	int32 Aiming = 0.8;


};
