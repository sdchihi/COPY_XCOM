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
	* 애니메이션을 재생합니다.
	* @param AnimationName - 재생할 Animation의 이름
	* @return 재생할 Animation의 길이
	*/
	float PlayAnimationByName(FName AnimationName);

protected:
	/** 재생 가능한 Widget Animation 목록을 저장할 Map */
	TMap<FName, class UWidgetAnimation*> AnimationsMap;

	/**
	* 애니메이션을 얻어옵니다.
	* @param AnimationName - Animation의 이름
	* @return Animation 포인터
	*/
	UWidgetAnimation* GetAnimationByName(FName AnimationName) const;

	/** 재생 가능한 Widget Animation 목록을 Map에 얻어옵니다. */
	void FillAnimationsMap();

private:
	
	
};
