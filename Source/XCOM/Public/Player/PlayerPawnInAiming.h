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

	/** 후방으로 이격 거리 */
	UPROPERTY(EditDefaultsOnly)
	float BackWardDistance = 100;

	/** 우측으로 이격 거리 */
	UPROPERTY(EditDefaultsOnly)
	float RightDistance = 100;

	/** 윗 방향으로 이격 거리 */
	UPROPERTY(EditDefaultsOnly)
	float UpwardDistance = 100;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	/**
	* 조준하는 상황에서 사용될 Pawn의 카메라의 위치, 방향을 결정합니다. ( 캐릭터의 뒷면에서 )
	* @param AimingCharLoc - 조준하는 캐릭터의 위치
	* @param AimedCharLoc - 조준 대상이되는 캐릭터의 위치
	*/
	void SetCameraPositionInAimingSituation(const FVector AimingCharLoc, const FVector AimedCharLoc);

	/**
	* 조준하는 상황에서 사용될 Pawn의 카메라의 위치, 방향을 결정합니다. ( 캐릭터의 정면에서 )
	* @param AimingCharLoc - 조준하는 캐릭터의 위치
	* @param AimedCharLoc - 조준 대상이되는 캐릭터의 위치
	*/
	void SetShootingCam(const FVector AimingCharLoc, const FVector AimedCharLoc);

	/**
	* 캐릭터가 죽을때 사용될 Pawn의 카메라의 위치, 방향을 결정합니다.
	* @param AimingCharLoc - 조준하는 캐릭터의 위치
	* @param MurderedActor - 죽게되는 Actor
	*/
	void SetDeathCam(const FVector AimingCharLoc, AActor* MurderedActor);

	/**
	* 정면에서 캐릭터를 서서히 클로즈업시킬때 Pawn의 카메라의 위치, 방향을 결정합니다.
	* @param TargetActor - 촬영될 Actor
	* @param ForwardDirction - 정면 방향의 Dircetion
	*/
	void SetCloseUpCam(AActor* TargetActor, FVector ForwardDirction);

	/**
	* 정면에서 캐릭터를 찍을때 Pawn의 카메라의 위치, 방향을 결정합니다.
	* @param Actor 촬영될 Actor
	*/
	void SetFrontCam(AActor* Actor);

	/** 카메라 움직임을 멈춥니다. */
	void StopCameraMoving();
	
private:
	/** 유닛의 머리로 초점을 맞추는지 여부 */
	bool bFocusHead = false;

	USkeletalMeshComponent* BoneToFocus;

	AActor* ActorToFocus;

	/** 카메라의 움직임 시작 지점 */
	FVector StartLocation;

	/** 카메라의 움직임 중단 지점 */
	FVector EndLocation;

	/** 카메라의 이동 여부 */
	bool bCameraMoving = false;

	bool bNeedToChangeLocation = false;

	/**
	* 카메라의 시야에 방해되는 Actor가 존재하는지 확인합니다.
	* @param StartLocation - Trace 시작 위치
	* @param TargetLocation - Trace 종료 위치
	* @return 카메라 시야 방해 여부
	*/
	bool CheckInView(const FVector StartLocation, const FVector TargetLocation);

	/**
	* Actor 의 Head 위치를 얻어옵니다.
	* @return Actor 의 Head 위치
	*/
	FVector GetActorsHeadLocation()const;

	/**
	* 초점을 맞출 대상을 설정합니다
	* @param TargetActor - 초점을 맞출 Actor
	*/
	void SetFocusTarget(AActor* TargetActor);
};
