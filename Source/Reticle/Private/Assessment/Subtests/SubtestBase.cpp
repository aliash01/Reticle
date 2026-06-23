// Fill out your copyright notice in the Description page of Project Settings.

#include "Assessment/Subtests/SubtestBase.h"
#include "Assessment/AssessmentLog.h"
#include "Assessment/UI/PromptWidget.h"
#include "Assessment/UI/TrialProgressWidget.h"


void USubtestBase::Initialise(ASpawnManager* InSpawnManager, APawn* InPlayerPawn)
{
    ensureMsgf(InSpawnManager, TEXT("Initialise: null SpawnManager"));
    SpawnManager = InSpawnManager;
    PlayerPawn = InPlayerPawn;
}

void USubtestBase::BeginSubtest(USubtestConfigBase* Config)
{
    if (!ensureMsgf(Config, TEXT("BeginSubtest: null Config"))) return;
    
    SubtestConfig = Config->GetConfig();

    if (SubtestConfig.TrialProgressWidgetConfig.TrialProgressWidgetClass)
    {
        TrialProgressWidget = CreateWidget<UTrialProgressWidget>(GetWorld(), SubtestConfig.TrialProgressWidgetConfig.TrialProgressWidgetClass);
        TrialProgressWidget->SetMaxTrials(SubtestConfig.NumberOfTrials);
        TrialProgressWidget->AddToViewport();
        TrialProgressWidget->SetVisibility(ESlateVisibility::Collapsed);
    }
    else
    {
        UE_LOG(LogAssessment, Warning, TEXT("TrialProgressWidgetClass not set"));
    }

    if (SubtestConfig.PromptWidgetConfig.PromptWidgetClass)
    {
        PromptWidget = CreateWidget<UPromptWidget>(GetWorld(), SubtestConfig.PromptWidgetConfig.PromptWidgetClass);
        PromptWidget->AddToViewport();
        PromptWidget->SetVisibility(ESlateVisibility::Collapsed);
    }
    
    if (bSubtestRunning)
    {
        UE_LOG(LogAssessment, Warning, TEXT("BeginSubtest while already running - ignoring"));
        return;
    }
    
    if (!ensureMsgf(SpawnManager, TEXT("BeginSubtest before Initialise")))
    {
        return;
    }

    if (SubtestConfig.NumberOfTrials <= 0)
    {
        UE_LOG(LogAssessment, Warning, TEXT("NumberOfTrials <= 0")); return;
    }
    
    ensure(GetWorld());   // null world = wrong outer; everything downstream uses timers
    
    bSubtestRunning = true;
    UTCStartTime = FDateTime::UtcNow();
    RandomStream.Initialize(SubtestConfig.Seed);

    OnSubtestStart(Config);
    
    UE_LOG(LogAssessment, Log, TEXT("Subtest %s begin — session=%s seed=%d trials=%d"),
      *GetSubtestId().ToString(), *SubtestConfig.SessionId.ToString(), SubtestConfig.Seed, SubtestConfig.NumberOfTrials);

    BeginBetweenTrials();
}

void USubtestBase::StartTrial()
{
    if (bTrialActive) return;
    bTrialActive = true;

    if (TrialProgressWidget)
    {
        TrialProgressWidget->SetVisibility(ESlateVisibility::Collapsed);
    }
    if (PromptWidget)
    {
        PromptWidget->HidePrompt();   // clear the get-ready prompt before the trial
    }

    UE_LOG(LogAssessment, Verbose, TEXT("Trial %d start"), CurrentTrialIndex);   // StartTrial
    
    OnTrialStart();
}

void USubtestBase::SetTrialTimer(float Duration)
{
    ensureMsgf(Duration > 0.f, TEXT("Trial Time is invalid (<= 0.f)"));
    GetWorld()->GetTimerManager().SetTimer(
          TrialTimer,            
          this,
          &USubtestBase::EndTrial,  
          Duration,                
          false);
}

void USubtestBase::BeginBetweenTrials()
{
    // Three independently-gated phases run in order, each skipping straight to the next if
    // its widget isn't set or its time <= 0: round progress -> prompt -> blank gap -> trial.
    ShowRoundProgress();
}

void USubtestBase::ShowRoundProgress()
{
    const FTrialProgressWidgetConfig& Cfg = SubtestConfig.TrialProgressWidgetConfig;
    if (TrialProgressWidget && Cfg.Time > 0.f)
    {
        TrialProgressWidget->SetInstruction(Cfg.Instruction);
        TrialProgressWidget->UpdateCurrentRoundTick(CurrentTrialIndex);
        TrialProgressWidget->SetVisibility(ESlateVisibility::Visible);

        GetWorld()->GetTimerManager().SetTimer(
              BetweenTrialsTimer, this, &USubtestBase::ShowRoundPrompt, Cfg.Time, false);
        return;
    }

    ShowRoundPrompt();   // phase skipped
}

void USubtestBase::ShowRoundPrompt()
{
    if (TrialProgressWidget)
    {
        TrialProgressWidget->SetVisibility(ESlateVisibility::Collapsed);
    }

    const FPromptWidgetConfig& Cfg = SubtestConfig.PromptWidgetConfig;
    if (PromptWidget && Cfg.Time > 0.f)
    {
        // Duration 0 = manual hide; the timer below drives the handoff.
        PromptWidget->ShowPrompt(Cfg.Instruction, Cfg.BackgroundColour, 0.f);

        GetWorld()->GetTimerManager().SetTimer(
              BetweenTrialsTimer, this, &USubtestBase::ShowPreRoundBlank, Cfg.Time, false);
        return;
    }

    ShowPreRoundBlank();   // phase skipped
}

void USubtestBase::ShowPreRoundBlank()
{
    // Clear any prompt/progress so the last beat before the round is blank.
    if (PromptWidget) { PromptWidget->HidePrompt(); }
    if (TrialProgressWidget) { TrialProgressWidget->SetVisibility(ESlateVisibility::Collapsed); }

    if (SubtestConfig.BetweenTrialsTime > 0.f)
    {
        GetWorld()->GetTimerManager().SetTimer(
              BetweenTrialsTimer, this, &USubtestBase::StartTrial, SubtestConfig.BetweenTrialsTime, false);
        return;
    }

    StartTrial();   // no blank gap configured
}

void USubtestBase::EndTrial()
{
    if (!bTrialActive) return;
    bTrialActive = false;
    
    UE_LOG(LogAssessment, Verbose, TEXT("Trial %d end"),   CurrentTrialIndex);   // EndTrial, before increment
    
    GetWorld()->GetTimerManager().ClearTimer(TrialTimer);
    CurrentTrialIndex++;
    
    OnTrialEnd();
    
    if (CurrentTrialIndex >= SubtestConfig.NumberOfTrials)
    {
        EndSubtest(false);
    }
    else
    {
        BeginBetweenTrials();
    }
}

void USubtestBase::EndSubtest(bool bAborted)
{
    if (!bSubtestRunning)
    {
        UE_LOG(LogAssessment, Warning, TEXT("EndSubtest while not running — ignoring"));
        return;
    }

    OnSubtestEnd();
    
    // Clear both for cleanup/safety
    GetWorld()->GetTimerManager().ClearTimer(TrialTimer);
    GetWorld()->GetTimerManager().ClearTimer(BetweenTrialsTimer);

    // Remove this subtest's widgets so they don't accumulate in the viewport across a battery.
    if (TrialProgressWidget) { TrialProgressWidget->RemoveFromParent(); TrialProgressWidget = nullptr; }
    if (PromptWidget) { PromptWidget->RemoveFromParent(); PromptWidget = nullptr; }

    bSubtestRunning = false;
    OnSubtestEnded.Broadcast(GetSubtestResults(bAborted));

    UE_LOG(LogAssessment, Log, TEXT("Subtest %s end — %d/%d trials, aborted=%d"),
      *GetSubtestId().ToString(), CurrentTrialIndex, SubtestConfig.NumberOfTrials, bAborted);
}

FSubtestResult USubtestBase::GetSubtestResults(bool bAborted)
{
    FSubtestResult SubtestResult;
    SubtestResult.SessionId = SubtestConfig.SessionId;
    SubtestResult.SubtestId = GetSubtestId();
    SubtestResult.StartedAtUtc = UTCStartTime;
    SubtestResult.CompletedAtUtc = FDateTime::UtcNow();
    SubtestResult.TrialsCompleted = CurrentTrialIndex;
    SubtestResult.bAborted = bAborted; 
    SubtestResult.TrialRecordsJson = GetSubtestTrialRecordsJson();
    SubtestResult.AggregateJson = GetSubtestAggregateJson();
    return SubtestResult;
}   
bool USubtestBase::IsSubtestRunning() const
{
    return bSubtestRunning;
}

TArray<FString> USubtestBase::GetSubtestTrialRecordsJson()
{
    // in base is empty override in child subtest class
    return {};
}

FString USubtestBase::GetSubtestAggregateJson()
{
    return FString();
}

FName USubtestBase::GetSubtestId() const
{
    return FName(TEXT("Base")); 
}

