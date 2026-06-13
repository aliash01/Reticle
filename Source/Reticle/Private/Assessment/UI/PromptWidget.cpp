// Fill out your copyright notice in the Description page of Project Settings.

#include "Assessment/UI/PromptWidget.h"

#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

void UPromptWidget::ShowPrompt(const FText& Message, FLinearColor BackgroundColor, float Duration)
{
	if (MessageText)
	{
		MessageText->SetText(Message);
	}
	if (Background)
	{
		Background->SetBrushColor(BackgroundColor);
	}

	// Visible, but lets clicks pass through to the game — so a click during the
	// foreperiod still reaches the pawn's OnFire (a false start) instead of being
	// swallowed by the widget.
	SetVisibility(ESlateVisibility::HitTestInvisible);

	GetWorld()->GetTimerManager().ClearTimer(HideTimer);
	if (Duration > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			HideTimer, this, &UPromptWidget::HidePrompt, Duration, false);
	}
}

void UPromptWidget::HidePrompt()
{
	GetWorld()->GetTimerManager().ClearTimer(HideTimer);
	SetVisibility(ESlateVisibility::Collapsed);
}
