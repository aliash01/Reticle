// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TrialProgressWidget.generated.h"

/**
 *
 */
UCLASS()
class RETICLE_API UTrialProgressWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent)
	void BuildTicks();

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateCurrentRoundTick(int32 Round);

	virtual void NativeConstruct() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	int32 TotalTrials = 30;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	int32 CurrentTrial = 0;
};
