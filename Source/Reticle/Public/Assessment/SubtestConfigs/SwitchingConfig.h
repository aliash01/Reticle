// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SubtestConfigBase.h"
#include "SwitchingConfig.generated.h"

USTRUCT(BlueprintType)
struct FSwitchingSubtestConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 NumTargets = 4;                // targets on screen at once (>= 2); the active one jumps between them
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float SwitchTimeoutSeconds = 1.2f;   // miss the active target if not hit in time, then it switches anyway
};

/**
 *
 */
UCLASS()
class RETICLE_API USwitchingConfig : public USubtestConfigBase
{
	GENERATED_BODY()

public:
	FSwitchingSubtestConfig& GetSwitchingConfig() { return SwitchingConfig; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Switching Config")
	FSwitchingSubtestConfig SwitchingConfig;
};
