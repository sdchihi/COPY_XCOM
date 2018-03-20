// Fill out your copyright notice in the Description page of Project Settings.

#include "Path.h"

Path::Path() : bWall(false), bPawn(false), ParentIndex(INT_MAX), PathIndex(0), CostF(0), CostG(0), CostH(0), bCanMoveWithOneAct(false)
{
}

Path::Path(int32 index) : bWall(false), bPawn(false), ParentIndex(INT_MAX), PathIndex(index), CostF(0), CostG(0), CostH(0), bCanMoveWithOneAct(false)
{
}

Path::~Path()
{
}


void Path::Clear(bool bClearAll) {
	ParentIndex = INT_MAX;

	CostF = 0;

	CostG = 0;

	CostH = 0;

	PathDirection = 0;

	if (bClearAll) 
	{
		OnTheWay.Empty();
		OnTheWayMap.Empty();
	}

}