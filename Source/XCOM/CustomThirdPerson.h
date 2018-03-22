// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CustomThirdPerson.generated.h"

class AGun;
class USpringArmComponent;
class UCameraComponent;

DECLARE_DYNAMIC_DELEGATE(FChangePlayerPawnDelegate);

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
	
	TMap<ECoverDirection, bool> CoverDirectionMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AGun> GunBlueprint;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	AGun* GunReference;

	void CheckAttackPotential(APawn* TargetPawn);

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

	int32 CurrentMovableStep;
	
	int32 GetMovableStepPerActionPoint() { return Step / 2; };

	void RotateTowardWall();

	FChangePlayerPawnDelegate ChangePlayerPawnDelegate;

	UPROPERTY(EditDefaultsOnly)
	int32 RemainingActionPoint = 2;

	UFUNCTION(BlueprintImplementableEvent)
	void AimAt(FVector AimDirection);

	UFUNCTION(BlueprintCallable)
	void RotateCharacter(FVector AimDirection, float LerpAlpha);

	UFUNCTION(BlueprintCallable)
	void StartFiring();

	int32 GetStep() { return Step; };

	void UseActionPoint(int32 PointToUse);

protected:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser) override;

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* SpringArm= nullptr;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	bool bTeam = true;

	UPROPERTY(EditDefaultsOnly)
	int32 Step = 10;

	UPROPERTY(EditDefaultsOnly)
	float AttackRange = 500;

	float CalculateAttackSuccessRatio(const FHitResult HitResult, APawn* TargetPawn);

	float CalculateAngleBtwAimAndWall(const FVector AimDirection, ACustomThirdPerson* TargetPawn); 

	void SetOffAttackState();


};
