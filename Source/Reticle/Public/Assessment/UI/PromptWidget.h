// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PromptWidget.generated.h"

class UTextBlock;
class UBorder;

/**
 * Generic message/prompt screen for non-active periods (get-ready, rest,
 * instructions, feedback). The caller supplies the text, background colour,
 * and how long to show it; the WBP supplies the layout/design.
 */
UCLASS()
class RETICLE_API UPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Show the prompt with the given text + background colour.
	// Duration > 0  -> auto-hides after that many seconds.
	// Duration <= 0 -> stays up until HidePrompt() is called (e.g. the random foreperiod).
	UFUNCTION(BlueprintCallable, Category = "Prompt")
	void ShowPrompt(const FText& Message, FLinearColor BackgroundColor = FLinearColor(0,0,0,0), float Duration = 0.f);

	UFUNCTION(BlueprintCallable, Category = "Prompt")
	void HidePrompt();

protected:
	// Name a Text Block "MessageText" and a Border "Background" in the WBP to wire these.
	// Optional: if absent, that part is simply skipped (no crash).
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* MessageText;

	UPROPERTY(meta = (BindWidgetOptional))
	UBorder* Background;

private:
	FTimerHandle HideTimer;
};
