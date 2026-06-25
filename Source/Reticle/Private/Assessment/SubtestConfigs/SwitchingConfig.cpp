// Fill out your copyright notice in the Description page of Project Settings.


#include "Assessment/SubtestConfigs/SwitchingConfig.h"
#include "Assessment/Subtests/SwitchingSubtest.h"

TSubclassOf<USubtestBase> USwitchingConfig::GetSubtestClass() const
{
	return USwitchingSubtest::StaticClass();
}