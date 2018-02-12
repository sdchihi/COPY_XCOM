// Fill out your copyright notice in the Description page of Project Settings.

#include "Path.h"

Path::Path() : bWall(false), bPawn(false), ParentIndex(0), PathIndex(0), CostF(0), CostG(0), CostH(0)
{
}

Path::Path(int32 index) : bWall(false), bPawn(false), ParentIndex(0), PathIndex(index), CostF(0), CostG(0), CostH(0)
{
}

Path::~Path()
{
}

