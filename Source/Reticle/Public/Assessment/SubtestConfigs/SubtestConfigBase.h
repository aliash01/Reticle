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
};

USTRUCT(BlueprintType)
struct FPromptWidgetConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSubclassOf<UPromptWidget> PromptWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Instruction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor BackgroundColour;
};

USTRUCT(BlueprintType)
struct FSubtestConfig
{
	GENERATED_BODY()

	FGuid SessionId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Seed = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 NumberOfTrials = 30;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float TrialTime = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float BetweenTrialsTime = 0.5f;
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
