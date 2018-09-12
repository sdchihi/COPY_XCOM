// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/CustomUserWidget.h"
#include "UnitHUD.generated.h"

/**
 * 
 */
UCLASS()
class XCOM_API UUnitHUD : public UCustomUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	/**
	* Actor를 등록합니다.
	* @param ActorToSet - 등록할 Actor
	*/
	UFUNCTION(BlueprintCallable)
	void RegisterActor(class ACustomThirdPerson* ActorToSet);

	/** HP바를 초기화합니다.*/
	void InitializeHPBar();

	/**
	* 체력을 감소시킵니다.
	* @param Damage - 데미지 값
	*/
	void ReduceHP(int8 Damage);

	void DestroyHPBar();

private:
	UPROPERTY()
	ACustomThirdPerson* CharacterRef;

	UPROPERTY()
	class UUniformGridPanel* GridPannel;

	UPROPERTY()
	class UImage* TeamImage;

	UPROPERTY()
	UImage* Background;

	TArray <UCustomUserWidget* > HPWidgetArray;

	/** 팀을 표시하는 이미지를 갱신합니다. */
	void SetTeamIconImage();

	/**
	* 팀에 맞춰서 사용하게 될 HP Block 클래스를 얻어옵니다.
	* @param bIsPlayerTeam - 팀 플래그
	*/
	UClass* GetHPClass(bool bIsPlayerTeam);
};
