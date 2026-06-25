// Fill out your copyright notice in the Description page of Project Settings.


#include "Assessment/SubtestConfigs/FlickConfig.h"
#include "Assessment/Subtests/FlickSubtest.h"

TSubclassOf<USubtestBase> UFlickConfig::GetSubtestClass() const
{
	return UFlickSubtest::StaticClass();
}