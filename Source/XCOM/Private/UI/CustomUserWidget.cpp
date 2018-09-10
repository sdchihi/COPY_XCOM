// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomUserWidget.h"
#include "Animation/WidgetAnimation.h"
#include "MovieScene.h"

void UCustomUserWidget::NativeConstruct()
{
	FillAnimationsMap();
	Super::NativeConstruct();
}

/**
* 재생 가능한 Widget Animation 목록을 Map에 얻어옵니다.
*/
void UCustomUserWidget::FillAnimationsMap()
{
	AnimationsMap.Empty();

	UProperty* Prop = GetClass()->PropertyLink;

	// Run through all properties of this class to find any widget animations
	while (Prop != nullptr)
	{
		// Only interested in object properties
		if (Prop->GetClass() == UObjectProperty::StaticClass())
		{
			UObjectProperty* ObjProp = Cast<UObjectProperty>(Prop);

			// Only want the properties that are widget animations
			if (ObjProp->PropertyClass == UWidgetAnimation::StaticClass())
			{
				UObject* Obj = ObjProp->GetObjectPropertyValue_InContainer(this);

				UWidgetAnimation* WidgetAnim = Cast<UWidgetAnimation>(Obj);

				if (WidgetAnim != nullptr && WidgetAnim->MovieScene != nullptr)
				{
					FName AnimName = WidgetAnim->MovieScene->GetFName();
					AnimationsMap.Add(AnimName, WidgetAnim);
				}
			}
		}
		Prop = Prop->PropertyLinkNext;
	}
}

/**
* 애니메이션을 얻어옵니다.
* @param AnimationName - Animation의 이름
* @return Animation 포인터
*/
UWidgetAnimation* UCustomUserWidget::GetAnimationByName(FName AnimationName) const
{
	UWidgetAnimation* const* WidgetAnim = AnimationsMap.Find(AnimationName);
	if (WidgetAnim)
	{
		return *WidgetAnim;
	}

	return nullptr;
};

/**
* 애니메이션을 재생합니다.
* @param AnimationName - 재생할 Animation의 이름
* @return 재생할 Animation의 길이
*/
float UCustomUserWidget::PlayAnimationByName(FName AnimationName) 
{
	UWidgetAnimation* Anim = GetAnimationByName(AnimationName);
	if (Anim)
	{
		PlayAnimation(Anim);
		return Anim->GetEndTime();
	}
	return 0;
}