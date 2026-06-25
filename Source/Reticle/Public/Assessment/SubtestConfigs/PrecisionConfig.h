// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SubtestConfigBase.h"
#include "PrecisionConfig.generated.h"

USTRUCT(BlueprintType)
struct FPrecisionSubtestConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ResponseWindowSeconds = 1.5f;   // miss if the target isn't hit in time
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Distance = 60.f;                // fixed local radius from centre; only the direction is randomised (no distance variation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MinSizeScale = 0.1f;            // randomised uniform target scale — the precision difficulty variable
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxSizeScale = 0.4f;
};

/**
 *
 */
UCLASS()
class RETICLE_API UPrecisionConfig : public USubtestConfigBase
{
	GENERATED_BODY()

public:
	virtual TSubclassOf<USubtestBase> GetSubtestClass() const override;
	FPrecisionSubtestConfig& GetPrecisionConfig() { return PrecisionConfig; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Precision Config")
	FPrecisionSubtestConfig PrecisionConfig;
};
