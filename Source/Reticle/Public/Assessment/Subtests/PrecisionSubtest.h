// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Assessment/SubtestConfigs/PrecisionConfig.h"
#include "Assessment/Subtests/SubtestBase.h"
#include "Common/Target/Target.h"
#include "PrecisionSubtest.generated.h"

UENUM(BlueprintType)
enum class EPrecisionOutcome : uint8
{
	Hit,
	Miss
};

// One fired shot. Signed errors to the target centre are kept (not a single magnitude)
// so overshoot / directional-bias analysis stays possible downstream.
USTRUCT(BlueprintType)
struct FPrecisionShot
{
	GENERATED_BODY()

	UPROPERTY() int32 ShotIndex = 0;         // 0-based order within the trial
	UPROPERTY() float YawErrorDeg = 0.f;     // signed: + = target right of aim
	UPROPERTY() float PitchErrorDeg = 0.f;   // signed: + = target above aim
	UPROPERTY() int32 TimeMs = 0;            // ms since the target appeared
	UPROPERTY() bool bHit = false;           // did this shot connect
};

// Raw per-trial record. TargetSizeScale is the difficulty variable — accuracy-vs-size
// is derived downstream, not computed here.
USTRUCT(BlueprintType)
struct FPrecisionRoundResult
{
	GENERATED_BODY()

	UPROPERTY() int32 TrialIndex = 0;
	UPROPERTY() EPrecisionOutcome Outcome = EPrecisionOutcome::Miss;
	UPROPERTY() float TargetSizeScale = 0.f;                    // randomised uniform scale this trial
	UPROPERTY() FVector TargetLocalPos = FVector::ZeroVector;   // where it spawned (manager-local)
	UPROPERTY() TArray<FPrecisionShot> Shots;
};

USTRUCT(BlueprintType)
struct FPrecisionAggregate
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
class RETICLE_API UPrecisionSubtest : public USubtestBase
{
	GENERATED_BODY()

	virtual void Initialise(ASpawnManager* InSpawnManager, APawn* InPlayerPawn) override;
	virtual void OnSubtestStart(USubtestConfigBase* Config) override;
	virtual void OnSubtestEnd() override;
	virtual void OnTrialStart() override;
	virtual void OnTrialEnd() override;

	void OnPrecisionFire();   // every shot — records its accuracy

	UFUNCTION()
	void OnPrecisionHit(ATarget* HitTarget);    // a shot connected -> ends the trial
	UFUNCTION()
	void OnPrecisionMissed(ATarget* ExpiredTarget);   // timeout -> miss

	virtual FName GetSubtestId() const override;
	virtual TArray<FString> GetSubtestTrialRecordsJson() override;
	virtual FString GetSubtestAggregateJson() override;

	UPROPERTY()
	ATarget* Target;

	FPrecisionSubtestConfig PrecisionSubtestConfig;

	FPrecisionRoundResult CurrentRoundResult;
	TArray<FPrecisionRoundResult> RoundResults;
};
