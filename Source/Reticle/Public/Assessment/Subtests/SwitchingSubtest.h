// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Assessment/SubtestConfigs/SwitchingConfig.h"
#include "Assessment/Subtests/SubtestBase.h"
#include "Common/Target/Target.h"
#include "SwitchingSubtest.generated.h"

UENUM(BlueprintType)
enum class ESwitchOutcome : uint8
{
	Hit,
	Miss
};

// One fired shot during a switch. Signed errors are to the ACTIVE target's centre;
// bWrongTarget marks a shot that connected with a non-active target (a mis-switch).
USTRUCT(BlueprintType)
struct FSwitchShot
{
	GENERATED_BODY()

	UPROPERTY() int32 ShotIndex = 0;         // 0-based order within this switch (0 = first shot)
	UPROPERTY() float YawErrorDeg = 0.f;     // signed: + = active target was right of aim
	UPROPERTY() float PitchErrorDeg = 0.f;   // signed: + = active target was above aim
	UPROPERTY() int32 TimeMs = 0;            // ms since the target became active (reacquisition clock)
	UPROPERTY() bool bHit = false;           // connected with the active target
	UPROPERTY() bool bWrongTarget = false;   // connected with a non-active target (mis-switch)
};

// Raw per-switch record. Reacquisition time / first-shot error / shots-to-hit are
// derived downstream from the shots + SwitchAngleDeg, not computed here.
USTRUCT(BlueprintType)
struct FSwitchingRoundResult
{
	GENERATED_BODY()

	UPROPERTY() int32 TrialIndex = 0;             // 0-based switch number within the subtest
	UPROPERTY() ESwitchOutcome Outcome = ESwitchOutcome::Miss;
	UPROPERTY() int32 ActiveIndex = 0;            // which of the N targets was active
	UPROPERTY() FVector ActiveLocalPos = FVector::ZeroVector;   // active target position (manager-local)
	UPROPERTY() float SwitchAngleDeg = 0.f;       // angular distance from the previous active target (flick amplitude)
	UPROPERTY() TArray<FSwitchShot> Shots;
};

// Deliberately minimal — real accuracy/efficiency metrics are derived downstream.
USTRUCT(BlueprintType)
struct FSwitchingAggregate
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
class RETICLE_API USwitchingSubtest : public USubtestBase
{
	GENERATED_BODY()

	virtual void Initialise(ASpawnManager* InSpawnManager, APawn* InPlayerPawn) override;
	virtual void OnSubtestStart(USubtestConfigBase* Config) override;
	virtual void OnSubtestEnd() override;
	virtual void OnTrialStart() override;
	virtual void OnTrialEnd() override;

	void OnSwitchFire();   // every shot — records its accuracy to the active target

	UFUNCTION()
	void OnSwitchHit(ATarget* HitTarget);   // a shot connected with a target (active -> advance, other -> mis-switch)

	virtual FName GetSubtestId() const override;
	virtual TArray<FString> GetSubtestTrialRecordsJson() override;
	virtual FString GetSubtestAggregateJson() override;

	// Angular distance (deg) between two targets as seen from the player's view.
	float AngleBetweenTargetsDeg(int32 IndexA, int32 IndexB) const;

	UPROPERTY()
	TArray<ATarget*> Targets;   // the N persistent, always-visible targets

	int32 ActiveIndex = INDEX_NONE;         // current active target
	int32 PreviousActiveIndex = INDEX_NONE; // for the inter-target flick angle
	bool bSwitchActive = false;             // gates shot/hit capture to an in-progress switch

	FSwitchingSubtestConfig SwitchingSubtestConfig;

	FSwitchingRoundResult CurrentRoundResult;
	TArray<FSwitchingRoundResult> RoundResults;
};
