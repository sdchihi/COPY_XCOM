// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponCameraShake.h"

UWeaponCameraShake::UWeaponCameraShake() 
{
	OscillationDuration = 0.5;
	OscillationBlendInTime = 0.05f;
	OscillationBlendOutTime = 0.05f;

	RotOscillation.Pitch.Amplitude = 0.25f;
	RotOscillation.Pitch.Frequency = 0.5f;

	RotOscillation.Yaw.Amplitude = 1.0f;
	RotOscillation.Yaw.Frequency = 5.0f;
}