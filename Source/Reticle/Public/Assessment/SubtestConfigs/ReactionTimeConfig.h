// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SubtestConfigBase.h"
#include "ReactionTimeConfig.generated.h"

USTRUCT(BlueprintType)
struct FReactionTimeSubtestConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ForeperiodMinTime = 1.f;        // min random wait before the target appears
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ForeperiodMaxTime = 3.f;        // max random wait
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ResponseWindowSeconds = 2.f;    // time to react before it's a NoResponse (your TargetLifetime)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 FalseStartCap = 10;             // consecutive false starts before aborting the subtest
};

/**
 * 
 */
UCLASS()
class RETICLE_API UReactionTimeConfig : public USubtestConfigBase
{
	GENERATED_BODY()

public:
	FReactionTimeSubtestConfig& GetReactionTimeConfig() { return ReactionTimeConfig; };
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reaction Time Config")
	FReactionTimeSubtestConfig ReactionTimeConfig;
};
