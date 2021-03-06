// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DestructibleWall.h"

/**
 * 
 */
class XCOM_API Path
{
public:
	Path() ;

	Path(int32 index);

	~Path();

	bool bWall;

	bool bPawn;

	int32 ParentIndex;

	int32 PathIndex;

	// F = G + H
	int32 CostF;

	int32 CostG;

	int32 CostH;

	TArray<int32> OnTheWay;

	TMap<int32, float> OnTheWayMap;

	float PathDirection;

	bool bCanMoveWithOneAct;

	void Clear(bool bClearAll = false);

	ECoverInfo CoverInfo = ECoverInfo::None;

};
