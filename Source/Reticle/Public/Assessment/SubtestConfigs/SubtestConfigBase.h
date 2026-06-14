// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Templates/SubclassOf.h"
#include "SubtestConfigBase.generated.h"

class UPromptWidget;
class UTrialProgressWidget;

USTRUCT(BlueprintType)
struct FTrialProgressWidgetConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSubclassOf<UTrialProgressWidget> TrialProgressWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Instruction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Time = 0.f;   // seconds to show the round-progress screen between rounds; 0 = skip this phase
};

USTRUCT(BlueprintType)
struct FPromptWidgetConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSubclassOf<UPromptWidget> PromptWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Instruction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor BackgroundColour;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Time = 0.f;   // seconds to show this prompt between rounds; 0 = skip this phase
};

USTRUCT(BlueprintType)
struct FSubtestConfig
{
	GENERATED_BODY()

	FGuid SessionId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Seed = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 NumberOfTrials = 30;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float TrialTime = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float BetweenTrialsTime = 0.5f;   // seconds of blank screen right before each round starts; 0 = none
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FTrialProgressWidgetConfig TrialProgressWidgetConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FPromptWidgetConfig PromptWidgetConfig;
};

/**
 * 
 */
UCLASS()
class RETICLE_API USubtestConfigBase : public UDataAsset
{
	GENERATED_BODY()

public:
	FSubtestConfig& GetConfig() { return Config; };
	 
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Config")
	FSubtestConfig Config;
};
