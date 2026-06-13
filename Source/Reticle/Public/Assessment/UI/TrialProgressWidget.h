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

	void SetMaxTrials(int32 Trials)
	{
		TotalTrials = Trials;
	}
	
protected:
	UFUNCTION(BlueprintCallable, Category = "Utilities|String")
	static FString PadNumberWithZeros(int32 Number, int32 MaxRounds)
	{
		int32 MinDigits = FString::FromInt(MaxRounds).Len();
		return FString::Printf(TEXT("%0*d"), MinDigits, Number);
	}

	virtual void NativeConstruct() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	int32 TotalTrials = 30;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	int32 CurrentTrial = 0;
};
