// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Assessment/SubtestConfigs/TrackingConfig.h"
#include "Assessment/Subtests/SubtestBase.h"
#include "Common/Target/Target.h"
#include "TrackingSubtest.generated.h"

class AAssessmentPawn;

// One fixed-rate sample of the player's tracking. Signed errors are to the target
// centre; bOnTarget is whether the crosshair was actually over the hitbox.
USTRUCT(BlueprintType)
struct FTrackingSample
{
	GENERATED_BODY()

	UPROPERTY() int32 TimeMs = 0;            // ms since the trial started
	UPROPERTY() float YawErrorDeg = 0.f;     // signed: + = target right of aim
	UPROPERTY() float PitchErrorDeg = 0.f;   // signed: + = target above aim
	UPROPERTY() bool bOnTarget = false;      // crosshair ray hit the target's hitbox
	UPROPERTY() bool bFiring = false;        // fire button held at this instant
};

// Raw per-trial record — the full sampled time-series. Mean error / time-on-target
// ratio are derived downstream, not here.
USTRUCT(BlueprintType)
struct FTrackingRoundResult
{
	GENERATED_BODY()

	UPROPERTY() int32 TrialIndex = 0;
	UPROPERTY() TArray<FTrackingSample> Samples;
};

USTRUCT(BlueprintType)
struct FTrackingAggregate
{
	GENERATED_BODY()

	UPROPERTY() int32 TrialCount = 0;
	UPROPERTY() int32 TotalSamples = 0;
};

// Internal path state for one axis: two seeded sine components, evaluated by time.
struct FAxisWave
{
	float A1 = 0.f, F1 = 0.f, P1 = 0.f;
	float A2 = 0.f, F2 = 0.f, P2 = 0.f;

	float Evaluate(float TimeSeconds) const;
};

/**
 *
 */
UCLASS()
class RETICLE_API UTrackingSubtest : public USubtestBase
{
	GENERATED_BODY()

	virtual void Initialise(ASpawnManager* InSpawnManager, APawn* InPlayerPawn) override;
	virtual void OnSubtestStart(USubtestConfigBase* Config) override;
	virtual void OnSubtestEnd() override;
	virtual void OnTrialStart() override;
	virtual void OnTrialEnd() override;

	void SampleTick();          // fixed-rate: move the target + record one sample
	FAxisWave MakeWave();       // seeded path for one axis

	virtual FName GetSubtestId() const override;
	virtual TArray<FString> GetSubtestTrialRecordsJson() override;
	virtual FString GetSubtestAggregateJson() override;

	UPROPERTY()
	ATarget* Target;

	// Cached in Initialise so the per-sample tick reads fire state without re-casting the pawn.
	UPROPERTY()
	AAssessmentPawn* AssessmentPawn = nullptr;

	FTrackingSubtestConfig TrackingSubtestConfig;

	FAxisWave YawWave;     // horizontal (local Y)
	FAxisWave PitchWave;   // vertical (local Z)

	FTimerHandle SampleTimer;

	FTrackingRoundResult CurrentRoundResult;
	TArray<FTrackingRoundResult> RoundResults;
};
