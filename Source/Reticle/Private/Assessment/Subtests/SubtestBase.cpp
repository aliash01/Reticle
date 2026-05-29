// Fill out your copyright notice in the Description page of Project Settings.

#include "Assessment/AssessmentLog.h"
#include "Assessment/Subtests/SubtestBase.h"

void USubtestBase::Initialise(ASpawnManager* InSpawnManager)
{
    ensureMsgf(InSpawnManager, TEXT("Initialise: null SpawnManager"));
    SpawnManager = InSpawnManager;
}

void USubtestBase::BeginSubtest(const FSubtestConfig& InSubtestConfig)
{
    if (bSubtestRunning)
    {
        UE_LOG(LogAssessment, Warning, TEXT("BeginSubtest while already running - ignoring"));
        return;
    }
    
    if (!ensureMsgf(SpawnManager, TEXT("BeginSubtest before Initialise")))
    {
        return;
    }

    if (InSubtestConfig.NumberOfTrials <= 0)
    {
        UE_LOG(LogAssessment, Warning, TEXT("NumberOfTrials <= 0")); return;
    }
    
    ensure(GetWorld());   // null world = wrong outer; everything downstream uses timers
    
    bSubtestRunning = true;
    UTCStartTime = FDateTime::UtcNow();
    SubtestConfig = InSubtestConfig;
    RandomStream.Initialize(SubtestConfig.Seed);

    UE_LOG(LogAssessment, Log, TEXT("Subtest %s begin — session=%s seed=%d trials=%d"),
      *GetSubtestId().ToString(), *SubtestConfig.SessionId.ToString(), SubtestConfig.Seed, SubtestConfig.NumberOfTrials);

    StartTrial();
}

void USubtestBase::StartTrial()
{
    if (bTrialActive) return;
    bTrialActive = true;
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

void USubtestBase::SetBetweenTrialsTimer(float Duration)
{
    ensureMsgf(Duration > 0.f, TEXT("Between Trials Time is invalid (<= 0.f)"));
    GetWorld()->GetTimerManager().SetTimer(
          BetweenTrialsTimer,            
          this,
          &USubtestBase::StartTrial,
          Duration,
          false
    );
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
        SetBetweenTrialsTimer(SubtestConfig.BetweenTrialsTime);
    }
}

void USubtestBase::EndSubtest(bool bAborted)
{
    if (!bSubtestRunning)
    {
        UE_LOG(LogAssessment, Warning, TEXT("EndSubtest while not running — ignoring"));
        return;
    }
    
    // Clear both for cleanup/safety
    GetWorld()->GetTimerManager().ClearTimer(TrialTimer);
    GetWorld()->GetTimerManager().ClearTimer(BetweenTrialsTimer);
    
    bSubtestRunning = false;
    OnSubtestEnd.Broadcast(GetSubtestResults(bAborted));

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

