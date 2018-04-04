// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomButton.h"

UCustomButton::UCustomButton()
{
	TScriptDelegate<FWeakObjectPtr> delegateFunction;
	delegateFunction.BindUFunction(this, FName("ConveyButtonIndex"));
	OnClicked.Add(delegateFunction);
}

void UCustomButton::ConveyButtonIndex()
{
	if (ConveyIndexDelegate.IsBound()) 
	{
		ConveyIndexDelegate.Execute(ButtonIndex);
	}
}