// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Assessment/SubtestConfigs/FlickConfig.h"
#include "Assessment/Subtests/SubtestBase.h"
#include "Common/Target/Target.h"
#include "FlickSubtest.generated.h"

UENUM(BlueprintType)
enum class EFlickOutcome : uint8
{
	Hit,
	Miss
};

// One fired shot. Signed errors are kept (not a single total angle) so that
// overshoot / directional-bias analysis remains possible downstream.
USTRUCT(BlueprintType)
struct FFlickShot
{
	GENERATED_BODY()

	UPROPERTY() int32 ShotIndex = 0;         // 0-based order within the trial (0 = first shot)
	UPROPERTY() float YawErrorDeg = 0.f;     // signed: + = target was right of aim
	UPROPERTY() float PitchErrorDeg = 0.f;   // signed: + = target was above aim
	UPROPERTY() int32 TimeMs = 0;            // ms since the target appeared
	UPROPERTY() bool bHit = false;           // did this shot connect
};

// Raw per-trial record. Metric derivation (first-shot error, shots-to-hit, etc.)
// is intentionally left to the analysis layer — this just stores the shots.
USTRUCT(BlueprintType)
struct FFlickRoundResult
{
	GENERATED_BODY()

	UPROPERTY() int32 TrialIndex = 0;          // 0-based trial/target number within the subtest
	UPROPERTY() EFlickOutcome Outcome = EFlickOutcome::Miss;
	UPROPERTY() FVector TargetLocalPos = FVector::ZeroVector;   // where the target spawned (manager-local) — the flick direction/distance
	UPROPERTY() TArray<FFlickShot> Shots;
};

// Deliberately minimal — real accuracy/efficiency metrics are derived downstream
// from the per-shot data, not computed here.
USTRUCT(BlueprintType)
struct FFlickAggregate
{
	GENERATED_BODY()

	UPROPERTY() int32 HitCount = 0;
	UPROPERTY() int32 MissCount = 0;
	UPROPERTY() int32 TotalShots = 0;
};

/**
 *
 */
UCLASS()
class RETICLE_API UFlickSubtest : public USubtestBase
{
	GENERATED_BODY()

	virtual void Initialise(ASpawnManager* InSpawnManager, APawn* InPlayerPawn) override;
	virtual void OnSubtestStart(USubtestConfigBase* Config) override;
	virtual void OnSubtestEnd() override;
	virtual void OnTrialStart() override;
	virtual void OnTrialEnd() override;

	void OnFlickFire();   // every shot — records its accuracy

	UFUNCTION()
	void OnFlickHit(ATarget* HitTarget);   // a shot connected -> ends the trial
	UFUNCTION()
	void OnFlickMissed(ATarget* ExpiredTarget);            // timeout -> miss

	virtual FName GetSubtestId() const override;
	virtual TArray<FString> GetSubtestTrialRecordsJson() override;
	virtual FString GetSubtestAggregateJson() override;

	UPROPERTY()
	ATarget* Target;

	float ResponseWindowSeconds = 1.5f;

	FFlickRoundResult CurrentRoundResult;
	TArray<FFlickRoundResult> RoundResults;

	UPROPERTY()
	FFlickSubtestConfig FlickSubtestConfig;
};
