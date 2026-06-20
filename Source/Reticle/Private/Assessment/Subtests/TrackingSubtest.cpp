// Fill out your copyright notice in the Description page of Project Settings.

#include "Assessment/Subtests/TrackingSubtest.h"

#include "JsonObjectConverter.h"
#include "Assessment/AssessmentLog.h"
#include "Assessment/AssessmentPawn.h"
#include "Assessment/SubtestConfigs/TrackingConfig.h"
#include "Common/SpawnManager.h"
#include "Common/Target/Target.h"
#include "GameFramework/Controller.h"

namespace
{
	constexpr float TraceRange = 100000.f;   // far enough to always reach the target
}

float FAxisWave::Evaluate(float TimeSeconds) const
{
	return A1 * FMath::Sin(2.f * PI * F1 * TimeSeconds + P1)
	     + A2 * FMath::Sin(2.f * PI * F2 * TimeSeconds + P2);
}

FAxisWave UTrackingSubtest::MakeWave()
{
	// Split the amplitude budget across two harmonics so |offset| <= PathAmplitude.
	FAxisWave W;
	W.A1 = RandomStream.FRandRange(0.4f, 0.7f) * TrackingSubtestConfig.PathAmplitude;
	W.A2 = TrackingSubtestConfig.PathAmplitude - W.A1;
	W.F1 = RandomStream.FRandRange(TrackingSubtestConfig.PathMinFreqHz, TrackingSubtestConfig.PathMaxFreqHz);
	W.F2 = RandomStream.FRandRange(TrackingSubtestConfig.PathMinFreqHz, TrackingSubtestConfig.PathMaxFreqHz);
	W.P1 = RandomStream.FRandRange(0.f, 2.f * PI);
	W.P2 = RandomStream.FRandRange(0.f, 2.f * PI);
	return W;
}

void UTrackingSubtest::PickReactiveVelocity()
{
	// Random direction in the local Y/Z plane at a fixed speed, held for a random short
	// interval before being re-chosen — this is what makes the reactive path dart.
	const float Angle = RandomStream.FRandRange(0.f, 2.f * PI);
	ReactiveVel = FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * TrackingSubtestConfig.ReactiveSpeed;
	ReactiveTimeUntilChange = RandomStream.FRandRange(
		TrackingSubtestConfig.ReactiveChangeMinSeconds, TrackingSubtestConfig.ReactiveChangeMaxSeconds);
}

void UTrackingSubtest::Initialise(ASpawnManager* InSpawnManager, APawn* InPlayerPawn)
{
	Super::Initialise(InSpawnManager, InPlayerPawn);

	// Cache the concrete pawn once; SampleTick reads IsFiring() without re-casting.
	AssessmentPawn = Cast<AAssessmentPawn>(InPlayerPawn);

	if (SpawnManager)
	{
		Target = SpawnManager->SpawnTarget(FVector::ZeroVector, 0.f);

		if (!Target)
		{
			UE_LOG(LogAssessment, Error, TEXT("Failed to spawn target"));
			return;
		}

		Target->Deactivate();
	}
}

void UTrackingSubtest::OnSubtestStart(USubtestConfigBase* Config)
{
	Super::OnSubtestStart(Config);

	UTrackingConfig* TrackingCfg = Cast<UTrackingConfig>(Config);

	if (!TrackingCfg)
	{
		UE_LOG(LogAssessment, Error, TEXT("TrackingConfig cast failed"));
		EndSubtest(true);   // don't run a misconfigured subtest
		return;
	}

	TrackingSubtestConfig = TrackingCfg->GetTrackingConfig();

	// Tracking is hold-to-fire (full auto); the other subtests stay semi-auto.
	if (AssessmentPawn)
	{
		AssessmentPawn->SetAutomaticFire(true, TrackingSubtestConfig.FireIntervalSeconds);
	}
}

void UTrackingSubtest::OnSubtestEnd()
{
	Super::OnSubtestEnd();

	GetWorld()->GetTimerManager().ClearTimer(SampleTimer);

	// Restore semi-auto so a later subtest doesn't inherit full-auto.
	if (AssessmentPawn)
	{
		AssessmentPawn->SetAutomaticFire(false, 0.f);
	}

	if (SpawnManager)
	{
		SpawnManager->DestroyTarget(Target);
	}
}

void UTrackingSubtest::OnTrialStart()
{
	if (!Target) return;

	CurrentRoundResult.TrialIndex = RoundResults.Num();

	// Fresh path for this trial — smooth (sum-of-sines) or reactive (erratic), per config.
	if (TrackingSubtestConfig.PathMode == ETrackingPathMode::Reactive)
	{
		ReactivePos = FVector2D::ZeroVector;
		PickReactiveVelocity();
	}
	else
	{
		YawWave = MakeWave();
		PitchWave = MakeWave();
	}

	Target->Activate(FVector::ZeroVector, 0.f);   // shown; no auto-expiry — the trial timer ends it
	Stopwatch.Start();                            // path + sample clock

	// Fixed-rate sampling, independent of frame/refresh rate.
	const float Interval = 1.f / FMath::Max(1.f, TrackingSubtestConfig.SampleRateHz);
	GetWorld()->GetTimerManager().SetTimer(
		SampleTimer, this, &UTrackingSubtest::SampleTick, Interval, true);

	// End the trial after its fixed duration.
	SetTrialTimer(TrackingSubtestConfig.TrialDurationSeconds);
}

void UTrackingSubtest::SampleTick()
{
	if (!Target || !PlayerPawn) return;

	const float T = Stopwatch.ElapsedSeconds();

	// Move the target (local Y = horizontal, Z = vertical) per the active path model.
	if (TrackingSubtestConfig.PathMode == ETrackingPathMode::Reactive)
	{
		const float Dt = 1.f / FMath::Max(1.f, TrackingSubtestConfig.SampleRateHz);
		const float Amp = TrackingSubtestConfig.PathAmplitude;

		ReactiveTimeUntilChange -= Dt;
		if (ReactiveTimeUntilChange <= 0.f) { PickReactiveVelocity(); }

		ReactivePos += ReactiveVel * Dt;

		// Bounce off the amplitude bounds so the target stays in the trackable area.
		if (ReactivePos.X < -Amp) { ReactivePos.X = -Amp; ReactiveVel.X = -ReactiveVel.X; }
		else if (ReactivePos.X > Amp) { ReactivePos.X = Amp; ReactiveVel.X = -ReactiveVel.X; }
		if (ReactivePos.Y < -Amp) { ReactivePos.Y = -Amp; ReactiveVel.Y = -ReactiveVel.Y; }
		else if (ReactivePos.Y > Amp) { ReactivePos.Y = Amp; ReactiveVel.Y = -ReactiveVel.Y; }

		Target->SetActorRelativeLocation(FVector(0.f, ReactivePos.X, ReactivePos.Y));
	}
	else
	{
		Target->SetActorRelativeLocation(FVector(0.f, YawWave.Evaluate(T), PitchWave.Evaluate(T)));
	}

	const AController* Controller = PlayerPawn->GetController();
	if (!Controller) return;

	FVector ViewLoc;
	FRotator ViewRot;
	Controller->GetPlayerViewPoint(ViewLoc, ViewRot);

	// Signed angular error to the target centre (the error-over-time signal).
	const FVector ToTarget = (Target->GetActorLocation() - ViewLoc).GetSafeNormal();
	const FVector Local = ViewRot.UnrotateVector(ToTarget);   // X = forward, Y = right, Z = up

	FTrackingSample Sample;
	Sample.TimeMs = FMath::RoundToInt(T * 1000.0);
	Sample.YawErrorDeg = FMath::RadiansToDegrees(FMath::Atan2(Local.Y, Local.X));
	Sample.PitchErrorDeg = FMath::RadiansToDegrees(FMath::Atan2(Local.Z, Local.X));

	// On-target = the crosshair ray actually hits the target's hitbox (same trace as a shot).
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(PlayerPawn);
	const FVector TraceEnd = ViewLoc + ViewRot.Vector() * TraceRange;
	if (GetWorld()->LineTraceSingleByChannel(Hit, ViewLoc, TraceEnd, ECC_Visibility, Params))
	{
		Sample.bOnTarget = (Hit.GetActor() == Target);
	}

	if (AssessmentPawn)
	{
		Sample.bFiring = AssessmentPawn->IsFiring();
	}

	CurrentRoundResult.Samples.Add(Sample);
}

void UTrackingSubtest::OnTrialEnd()
{
	Super::OnTrialEnd();

	GetWorld()->GetTimerManager().ClearTimer(SampleTimer);   // stop sampling
	if (Target) { Target->Deactivate(); }                    // hide between trials

	RoundResults.Add(CurrentRoundResult);

	CurrentRoundResult = FTrackingRoundResult();
	Stopwatch.Stop();
	Stopwatch.Reset();
}

FName UTrackingSubtest::GetSubtestId() const
{
	return TrackingSubtestConfig.PathMode == ETrackingPathMode::Reactive
		? FName(TEXT("ReactiveTracking"))
		: FName(TEXT("Tracking"));
}

TArray<FString> UTrackingSubtest::GetSubtestTrialRecordsJson()
{
	TArray<FString> Out;
	for (const FTrackingRoundResult& R : RoundResults)
	{
		FString Json;
		FJsonObjectConverter::UStructToJsonObjectString(R, Json);
		Out.Add(Json);
	}
	return Out;
}

FString UTrackingSubtest::GetSubtestAggregateJson()
{
	// Minimal — mean error / time-on-target ratio are derived downstream from the samples.
	FTrackingAggregate Agg;
	for (const FTrackingRoundResult& R : RoundResults)
	{
		Agg.TrialCount++;
		Agg.TotalSamples += R.Samples.Num();
	}

	FString Json;
	FJsonObjectConverter::UStructToJsonObjectString(Agg, Json);
	return Json;
}
