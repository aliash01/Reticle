// Fill out your copyright notice in the Description page of Project Settings.


#include "Assessment/SubtestConfigs/TrackingConfig.h"
#include "Assessment/Subtests/TrackingSubtest.h"

// Both Smooth and Reactive tracking run on UTrackingSubtest; PathMode in the asset selects behaviour.
TSubclassOf<USubtestBase> UTrackingConfig::GetSubtestClass() const
{
	return UTrackingSubtest::StaticClass();
}