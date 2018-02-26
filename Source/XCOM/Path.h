// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


enum class EPathDirection : uint8 {
	West,
	East,
	North,
	South,
	NorthWest,
	NorthEast,
	SouthWest,
	SouthEast,
	None
};
/**
 * 
 */
class XCOM_API Path
{
public:
	Path() ;

	Path(int32 index);

	~Path();


	//Getter Setter 모두 쓸것같아서 public 으로 지정

	bool bWall;

	bool bPawn;

	int32 ParentIndex;

	int32 PathIndex;

	// F = G + H
	int32 CostF;

	int32 CostG;

	int32 CostH;

	TArray<int32> OnTheWay;

	TMap<int32, EPathDirection> OnTheWayMap;

	EPathDirection PathDirection; 


	void Clear(bool bClearAll = false);
};
