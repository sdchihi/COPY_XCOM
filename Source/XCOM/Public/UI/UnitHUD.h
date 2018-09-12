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
	* Actor�� ����մϴ�.
	* @param ActorToSet - ����� Actor
	*/
	UFUNCTION(BlueprintCallable)
	void RegisterActor(class ACustomThirdPerson* ActorToSet);

	/** HP�ٸ� �ʱ�ȭ�մϴ�.*/
	void InitializeHPBar();

	/**
	* ü���� ���ҽ�ŵ�ϴ�.
	* @param Damage - ������ ��
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

	/** ���� ǥ���ϴ� �̹����� �����մϴ�. */
	void SetTeamIconImage();

	/**
	* ���� ���缭 ����ϰ� �� HP Block Ŭ������ ���ɴϴ�.
	* @param bIsPlayerTeam - �� �÷���
	*/
	UClass* GetHPClass(bool bIsPlayerTeam);
};
