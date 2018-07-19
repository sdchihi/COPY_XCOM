// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomUserWidget.h"
#include "Animation/WidgetAnimation.h"
#include "MovieScene.h"

void UCustomUserWidget::NativeConstruct()
{
	FillAnimationsMap();
	Super::NativeConstruct();
}

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

UWidgetAnimation* UCustomUserWidget::GetAnimationByName(FName AnimationName) const
{
	UWidgetAnimation* const* WidgetAnim = AnimationsMap.Find(AnimationName);
	if (WidgetAnim)
	{
		return *WidgetAnim;
	}

	return nullptr;
};

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