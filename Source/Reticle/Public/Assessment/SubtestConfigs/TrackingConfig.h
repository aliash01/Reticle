// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SubtestConfigBase.h"
#include "TrackingConfig.generated.h"

UENUM(BlueprintType)
enum class ETrackingPathMode : uint8
{
	Smooth,    // sum of two seeded sine harmonics per axis (predictable pursuit)
	Reactive   // erratic: random velocity held for a random interval, then re-chosen
};

USTRUCT(BlueprintType)
struct FTrackingSubtestConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) float TrialDurationSeconds = 10.f;   // length of each tracking trial
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float SampleRateHz = 60.f;           // fixed sample cadence (refresh-independent)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float FireIntervalSeconds = 0.05f;   // full-auto fire rate while the trigger is held

	// On-target is decided by tracing the crosshair against the target's hitbox
	// (same as a shot), so no angular threshold is needed here.

	// Path = sum of two seeded sine harmonics per axis. The seed draws frequencies
	// and phases from these ranges; PathAmplitude bounds the local excursion (Y/Z).
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float PathAmplitude = 150.f;         // max local-space excursion (world units)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float PathMinFreqHz = 0.10f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float PathMaxFreqHz = 0.50f;

	// Smooth uses the sum-of-sines above. Reactive ignores the freq ranges and instead darts
	// at ReactiveSpeed, re-randomising direction every Change[Min..Max]s, bounded by PathAmplitude.
	// Default Smooth keeps existing Tracking config assets behaving exactly as before.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) ETrackingPathMode PathMode = ETrackingPathMode::Smooth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ReactiveSpeed = 250.f;            // units/sec (reactive mode)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ReactiveChangeMinSeconds = 0.15f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ReactiveChangeMaxSeconds = 0.5f;
};

/**
 *
 */
UCLASS()
class RETICLE_API UTrackingConfig : public USubtestConfigBase
{
	GENERATED_BODY()

public:
	FTrackingSubtestConfig& GetTrackingConfig() { return TrackingConfig; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tracking Config")
	FTrackingSubtestConfig TrackingConfig;
};
