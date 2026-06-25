// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SubtestConfigBase.h"
#include "FlickConfig.generated.h"

USTRUCT(BlueprintType)
struct FFlickSubtestConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ResponseWindowSeconds = 1.5f;   // miss if the target isn't hit in time
};

/**
 *
 */
UCLASS()
class RETICLE_API UFlickConfig : public USubtestConfigBase
{
	GENERATED_BODY()

public:
	virtual TSubclassOf<USubtestBase> GetSubtestClass() const override;
	FFlickSubtestConfig& GetFlickConfig() { return FlickConfig; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flick Config")
	FFlickSubtestConfig FlickConfig;
};
