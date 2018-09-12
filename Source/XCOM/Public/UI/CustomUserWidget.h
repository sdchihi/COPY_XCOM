// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class XCOM_API UCustomUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	/**
	* �ִϸ��̼��� ����մϴ�.
	* @param AnimationName - ����� Animation�� �̸�
	* @return ����� Animation�� ����
	*/
	float PlayAnimationByName(FName AnimationName);

protected:
	/** ��� ������ Widget Animation ����� ������ Map */
	TMap<FName, class UWidgetAnimation*> AnimationsMap;

	/**
	* �ִϸ��̼��� ���ɴϴ�.
	* @param AnimationName - Animation�� �̸�
	* @return Animation ������
	*/
	UWidgetAnimation* GetAnimationByName(FName AnimationName) const;

	/** ��� ������ Widget Animation ����� Map�� ���ɴϴ�. */
	void FillAnimationsMap();

private:
	
	
};
