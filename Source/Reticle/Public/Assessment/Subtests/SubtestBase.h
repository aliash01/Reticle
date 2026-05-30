// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Assessment/SubtestConfigs/SubtestConfigBase.h"
#include "Common/Stopwatch.h"
#include "UObject/Object.h"
#include "SubtestBase.generated.h"
class ASpawnManager;



USTRUCT(BlueprintType)
struct FSubtestResult
{
	GENERATED_BODY()

	UPROPERTY() FString SchemaVersion = TEXT("1.0");   // PRD §8 — mandatory from day one
	UPROPERTY() FGuid   SessionId;
	UPROPERTY() FName   SubtestId;
	UPROPERTY() FDateTime StartedAtUtc;
	UPROPERTY() FDateTime CompletedAtUtc;
	UPROPERTY() int32   TrialsCompleted = 0;
	UPROPERTY() bool    bAborted = false;
	UPROPERTY() TArray<FString> TrialRecordsJson;      // opaque, subtest-defined
	UPROPERTY() FString AggregateJson;                 // opaque, subtest-defined (mean/SD/etc.)
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSubtestEndSignature, const FSubtestResult&)

/**
 * Base class for subtests, responsible for managing the execution of configurable trials
 * and aggregating results. Handles initialization, start, management, and termination
 * of subtests and their associated trials.
 */
UCLASS()
class RETICLE_API USubtestBase : public UObject
{
	GENERATED_BODY()

public:
	virtual void Initialise(ASpawnManager* InSpawnManager, APawn* InPlayerPawn);
	void BeginSubtest(USubtestConfigBase* Config);
	void EndSubtest(bool bAborted);
	

	FSubtestResult GetSubtestResults(bool bAborted);
	
	bool IsSubtestRunning() const;

	FOnSubtestEndSignature OnSubtestEnded;

	virtual UWorld* GetWorld() const override { return GetOuter() ? GetOuter()->GetWorld() : nullptr; }
protected:
	FSubtestConfig SubtestConfig;

	virtual TArray<FString> GetSubtestTrialRecordsJson();
	virtual FString GetSubtestAggregateJson();
	virtual FName GetSubtestId() const;
	virtual void OnTrialStart() {}
	virtual void OnTrialEnd() {}
	virtual void OnSubtestStart(USubtestConfigBase* Config) {}
	virtual void OnSubtestEnd() {}
	
	void StartTrial();
	void SetTrialTimer(float Duration);
	void SetBetweenTrialsTimer(float Duration);
	void EndTrial();
	
	UPROPERTY()
	ASpawnManager* SpawnManager;
	UPROPERTY() APawn* PlayerPawn;

	FRandomStream RandomStream;

	FStopwatch Stopwatch;
private:
	UPROPERTY()
	int32 CurrentTrialIndex = 0;

	UPROPERTY()
	FDateTime UTCStartTime;

	bool bSubtestRunning = false;
	bool bTrialActive = false;

	FTimerHandle TrialTimer;
	FTimerHandle BetweenTrialsTimer;
	
};
