// Fill out your copyright notice in the Description page of Project Settings.


#include "Assessment/Subtests/ReactionTimeSubtest.h"

#include "JsonObjectConverter.h"
#include "Assessment/AssessmentLog.h"
#include "Assessment/AssessmentPawn.h"
#include "Assessment/SubtestConfigs/ReactionTimeConfig.h"
#include "Assessment/UI/PromptWidget.h"
#include "Common/SpawnManager.h"
#include "Common/Target/Target.h"

void UReactionTimeSubtest::OnTrialStart()
{
	const float ForeperiodTime = RandomStream.FRandRange(ForeperiodMinTime, ForeperiodMaxTime);
	CurrentRoundResult.ForePeriodMs = ForeperiodTime * 1000.f;

	if (PromptWidget)
	{
		PromptWidget->ShowPrompt(SubtestConfig.PromptWidgetConfig.Instruction, SubtestConfig.PromptWidgetConfig.BackgroundColour, ForeperiodTime);
	}
	
	bInForeperiod = true;
	
	GetWorld()->GetTimerManager().SetTimer(
		  ForeperiodTimer,            
		  this,
		  &UReactionTimeSubtest::ShowTarget,
		  ForeperiodTime,
		  false
	);
}

void UReactionTimeSubtest::OnFalseStart()
{
	CurrentRoundResult.NumFalseStarts++;
	FalseStartsCount++;

	bInForeperiod = false;
		
	if (FalseStartsCount >= FalseStartsCap)
	{
		EndSubtest(true);
		return;
	}

	if (PromptWidget)
	{
		PromptWidget->ShowPrompt(RTSubtestConfig.FalseStartMessage, RTSubtestConfig.FalseStartBackgroundColour, RTSubtestConfig.FalseStartMessageDuration);
	}
	GetWorld()->GetTimerManager().SetTimer(
		  ForeperiodTimer,            
		  this,
		  &UReactionTimeSubtest::OnTrialStart,
		  RTSubtestConfig.FalseStartMessageDuration,
		  false
	);
}

void UReactionTimeSubtest::Initialise(ASpawnManager* InSpawnManager, APawn* InPlayerPawn)
{
	Super::Initialise(InSpawnManager, InPlayerPawn);

	if (AAssessmentPawn* AP = Cast<AAssessmentPawn>(InPlayerPawn))
	{
		AP->OnFire.AddUObject(this, &UReactionTimeSubtest::OnFire);
		AP->LockLook();
	}
	
	if (SpawnManager)
	{
		Target = SpawnManager->SpawnTarget(FVector::ZeroVector, TargetLifetime);

		if (!Target)
		{
			UE_LOG(LogAssessment, Error, TEXT("Failed to spawn target"));
			return;
		}
		
		Target->Deactivate();
		Target->OnTargetExpired.AddDynamic(this, &UReactionTimeSubtest::OnTrialTimeExpired);
		Target->OnTargetHit.AddDynamic(this, &UReactionTimeSubtest::OnTrialCompleted);
	}
}

void UReactionTimeSubtest::OnFire()
{
	if (!IsSubtestRunning()) return;
	if (bInForeperiod)
	{
		OnFalseStart();
	}
}


void UReactionTimeSubtest::ShowTarget()
{
	bInForeperiod = false;
	Target->Activate(FVector::ZeroVector, TargetLifetime);
	Stopwatch.Start();
}

void UReactionTimeSubtest::OnTrialTimeExpired(ATarget* ExpiredTarget)
{
	ExpiredTarget->Deactivate();
	
	CurrentRoundResult.Outcome = EReactionOutcome::NoResponse;

	EndTrial();
}

void UReactionTimeSubtest::OnTrialCompleted(ATarget* HitTarget)
{
	HitTarget->Deactivate();

	float TargetHitTime = Stopwatch.ElapsedSeconds();

	CurrentRoundResult.ReactionTimeMs = TargetHitTime * 1000.f;
	CurrentRoundResult.Outcome = EReactionOutcome::Valid;

	EndTrial();
}

void UReactionTimeSubtest::OnTrialEnd()
{
	Super::OnTrialEnd();

	RoundResults.Add(CurrentRoundResult);
	
	CurrentRoundResult = FReactionTimeRoundResult();
	Stopwatch.Stop();
	Stopwatch.Reset();
}

FName UReactionTimeSubtest::GetSubtestId() const
{
	return FName(TEXT("Reaction Time"));
}

void UReactionTimeSubtest::OnSubtestStart(USubtestConfigBase* Config)
{
	Super::OnSubtestStart(Config);

	UReactionTimeConfig* RTConfig = Cast<UReactionTimeConfig>(Config);

	if (!RTConfig)
	{
		UE_LOG(LogAssessment, Error, TEXT("ReactionTimeConfig cast failed"));
		EndSubtest(true);   // don't run a misconfigured subtest
		return;
	}
	
	RTSubtestConfig = RTConfig->GetReactionTimeConfig();
	
	TargetLifetime = RTSubtestConfig.ResponseWindowSeconds;
	ForeperiodMinTime = RTSubtestConfig.ForeperiodMinTime;
	ForeperiodMaxTime = RTSubtestConfig.ForeperiodMaxTime;
	FalseStartsCap = RTSubtestConfig.FalseStartCap;
}

void UReactionTimeSubtest::OnSubtestEnd()
{
	Super::OnSubtestEnd();

	if (AAssessmentPawn* AP = Cast<AAssessmentPawn>(PlayerPawn))
	{
		AP->OnFire.RemoveAll(this);
		AP->UnlockLook();
	}

	if (SpawnManager)
	{
		SpawnManager->DestroyTarget(Target);
	}
	
	GetWorld()->GetTimerManager().ClearTimer(ForeperiodTimer);
}

TArray<FString> UReactionTimeSubtest::GetSubtestTrialRecordsJson()
{
	TArray<FString> Out;
	for (const FReactionTimeRoundResult& R : RoundResults)
	{
		FString Json;
		FJsonObjectConverter::UStructToJsonObjectString(R, Json);
		Out.Add(Json);
	}
	return Out;
}

FString UReactionTimeSubtest::GetSubtestAggregateJson()
{
	FReactionTimeAggregate Agg;

	double Sum = 0.0;
	for (const FReactionTimeRoundResult& R : RoundResults)
	{
		Agg.TotalFalseStarts += R.NumFalseStarts;   // false starts can precede any outcome

		switch (R.Outcome)
		{
		case EReactionOutcome::Valid:
			Agg.ValidCount++;
			Sum += R.ReactionTimeMs;
			break;
		case EReactionOutcome::NoResponse:
			Agg.NoResponseCount++;
			break;
		default:
			break;
		}
	}

	if (Agg.ValidCount > 0)
		Agg.MeanRtMs = static_cast<float>(Sum / Agg.ValidCount);

	if (Agg.ValidCount > 1)   // sample SD (n-1); guard so we never divide by 0
	{
		double VarSum = 0.0;
		for (const FReactionTimeRoundResult& R : RoundResults)
			if (R.Outcome == EReactionOutcome::Valid)
			{
				const double Diff = R.ReactionTimeMs - Agg.MeanRtMs;
				VarSum += Diff * Diff;
			}
		Agg.SdRtMs = static_cast<float>(FMath::Sqrt(VarSum / (Agg.ValidCount - 1)));
	}

	FString Json;
	FJsonObjectConverter::UStructToJsonObjectString(Agg, Json);
	return Json;
}



