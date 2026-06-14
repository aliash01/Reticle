// Fill out your copyright notice in the Description page of Project Settings.


#include "Assessment/UI/TrialProgressWidget.h"

#include "Components/TextBlock.h"

void UTrialProgressWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BuildTicks();
}

void UTrialProgressWidget::SetInstruction(const FText& Instruction)
{
	if (InstructionText)
	{
		InstructionText->SetText(Instruction);
	}
}
