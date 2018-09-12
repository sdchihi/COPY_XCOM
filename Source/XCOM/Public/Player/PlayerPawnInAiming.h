// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerPawnInAiming.generated.h"

class UCameraComponent;

UCLASS()
class XCOM_API APlayerPawnInAiming : public APawn
{
	GENERATED_BODY()

public:
	APlayerPawnInAiming();

protected:
	virtual void BeginPlay() override;

public:	
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* Camera;

	/** �Ĺ����� �̰� �Ÿ� */
	UPROPERTY(EditDefaultsOnly)
	float BackWardDistance = 100;

	/** �������� �̰� �Ÿ� */
	UPROPERTY(EditDefaultsOnly)
	float RightDistance = 100;

	/** �� �������� �̰� �Ÿ� */
	UPROPERTY(EditDefaultsOnly)
	float UpwardDistance = 100;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	/**
	* �����ϴ� ��Ȳ���� ���� Pawn�� ī�޶��� ��ġ, ������ �����մϴ�. ( ĳ������ �޸鿡�� )
	* @param AimingCharLoc - �����ϴ� ĳ������ ��ġ
	* @param AimedCharLoc - ���� ����̵Ǵ� ĳ������ ��ġ
	*/
	void SetCameraPositionInAimingSituation(const FVector AimingCharLoc, const FVector AimedCharLoc);

	/**
	* �����ϴ� ��Ȳ���� ���� Pawn�� ī�޶��� ��ġ, ������ �����մϴ�. ( ĳ������ ���鿡�� )
	* @param AimingCharLoc - �����ϴ� ĳ������ ��ġ
	* @param AimedCharLoc - ���� ����̵Ǵ� ĳ������ ��ġ
	*/
	void SetShootingCam(const FVector AimingCharLoc, const FVector AimedCharLoc);

	/**
	* ĳ���Ͱ� ������ ���� Pawn�� ī�޶��� ��ġ, ������ �����մϴ�.
	* @param AimingCharLoc - �����ϴ� ĳ������ ��ġ
	* @param MurderedActor - �װԵǴ� Actor
	*/
	void SetDeathCam(const FVector AimingCharLoc, AActor* MurderedActor);

	/**
	* ���鿡�� ĳ���͸� ������ Ŭ�������ų�� Pawn�� ī�޶��� ��ġ, ������ �����մϴ�.
	* @param TargetActor - �Կ��� Actor
	* @param ForwardDirction - ���� ������ Dircetion
	*/
	void SetCloseUpCam(AActor* TargetActor, FVector ForwardDirction);

	/**
	* ���鿡�� ĳ���͸� ������ Pawn�� ī�޶��� ��ġ, ������ �����մϴ�.
	* @param Actor �Կ��� Actor
	*/
	void SetFrontCam(AActor* Actor);

	/** ī�޶� �������� ����ϴ�. */
	void StopCameraMoving();
	
private:
	/** ������ �Ӹ��� ������ ���ߴ��� ���� */
	bool bFocusHead = false;

	USkeletalMeshComponent* BoneToFocus;

	AActor* ActorToFocus;

	/** ī�޶��� ������ ���� ���� */
	FVector StartLocation;

	/** ī�޶��� ������ �ߴ� ���� */
	FVector EndLocation;

	/** ī�޶��� �̵� ���� */
	bool bCameraMoving = false;

	bool bNeedToChangeLocation = false;

	/**
	* ī�޶��� �þ߿� ���صǴ� Actor�� �����ϴ��� Ȯ���մϴ�.
	* @param StartLocation - Trace ���� ��ġ
	* @param TargetLocation - Trace ���� ��ġ
	* @return ī�޶� �þ� ���� ����
	*/
	bool CheckInView(const FVector StartLocation, const FVector TargetLocation);

	/**
	* Actor �� Head ��ġ�� ���ɴϴ�.
	* @return Actor �� Head ��ġ
	*/
	FVector GetActorsHeadLocation()const;

	/**
	* ������ ���� ����� �����մϴ�
	* @param TargetActor - ������ ���� Actor
	*/
	void SetFocusTarget(AActor* TargetActor);
};
