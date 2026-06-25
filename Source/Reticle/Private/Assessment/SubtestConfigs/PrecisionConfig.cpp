// Fill out your copyright notice in the Description page of Project Settings.


#include "Assessment/SubtestConfigs/PrecisionConfig.h"
#include "Assessment/Subtests/PrecisionSubtest.h"

TSubclassOf<USubtestBase> UPrecisionConfig::GetSubtestClass() const
{
	return UPrecisionSubtest::StaticClass();
}