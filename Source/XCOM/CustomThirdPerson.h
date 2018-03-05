// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CustomThirdPerson.generated.h"

class AGun;
class USpringArmComponent;
class UCameraComponent;

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


	void MoveToLocation(FVector Location);

	ECoverDirection CoverDirection = ECoverDirection::None;


	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	AGun* GunReference;

	void CheckAttackPotential(APawn* TargetPawn);

	bool GetTeamFlag() { return bTeam; };
	
private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* SpringArm= nullptr;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera = nullptr;
	

	UPROPERTY(EditDefaultsOnly)
	bool bTeam = true;

	UPROPERTY(EditDefaultsOnly)
	int step = 0;

	UPROPERTY(EditDefaultsOnly)
	int MaxHP = 100;

	UPROPERTY(EditDefaultsOnly)
	int CurrentHP;

	bool bCanAction = true;

	UPROPERTY(EditDefaultsOnly)
	float AttackRange = 500;

	float CalculateAttackSuccessRatio(FHitResult HitResult, APawn* TargetPawn);

};
