// Fill out your copyright notice in the Description page of Project Settings.


#include "Assessment/SubtestConfigs/ReactionTimeConfig.h"
#include "Assessment/Subtests/ReactionTimeSubtest.h"

TSubclassOf<USubtestBase> UReactionTimeConfig::GetSubtestClass() const
{
	return UReactionTimeSubtest::StaticClass();
}

