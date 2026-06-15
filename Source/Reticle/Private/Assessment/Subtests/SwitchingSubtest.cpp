// Fill out your copyright notice in the Description page of Project Settings.

#include "Assessment/Subtests/SwitchingSubtest.h"

#include "JsonObjectConverter.h"
#include "Components/SceneComponent.h"
#include "Assessment/AssessmentLog.h"
#include "Assessment/AssessmentPawn.h"
#include "Assessment/SubtestConfigs/SwitchingConfig.h"
#include "Common/SpawnManager.h"
#include "Common/Target/Target.h"
#include "GameFramework/Controller.h"

void USwitchingSubtest::Initialise(ASpawnManager* InSpawnManager, APawn* InPlayerPawn)
{
	Super::Initialise(InSpawnManager, InPlayerPawn);

	// Capture every shot (hit or miss), like Flick. The targets themselves are placed
	// in OnSubtestStart because their count comes from the config.
	if (AAssessmentPawn* AP = Cast<AAssessmentPawn>(InPlayerPawn))
	{
		AP->OnFire.AddUObject(this, &USwitchingSubtest::OnSwitchFire);
	}
}

void USwitchingSubtest::OnSubtestStart(USubtestConfigBase* Config)
{
	Super::OnSubtestStart(Config);

	USwitchingConfig* SwitchingCfg = Cast<USwitchingConfig>(Config);

	if (!SwitchingCfg)
	{
		UE_LOG(LogAssessment, Error, TEXT("SwitchingConfig cast failed"));
		EndSubtest(true);   // don't run a misconfigured subtest
		return;
	}

	SwitchingSubtestConfig = SwitchingCfg->GetSwitchingConfig();

	if (!SpawnManager) return;

	// Place N persistent, non-overlapping targets (seeded). They stay visible for the
	// whole subtest; only the highlight moves between switches.
	const int32 N = FMath::Max(2, SwitchingSubtestConfig.NumTargets);
	for (int32 i = 0; i < N; ++i)
	{
		const FVector Pos = SpawnManager->GeneratePosition(RandomStream);
		ATarget* T = SpawnManager->SpawnTarget(Pos, 0.f);   // LifeTime 0 = persistent, no auto-expiry
		if (!T)
		{
			UE_LOG(LogAssessment, Error, TEXT("Switching: failed to spawn target %d"), i);
			continue;
		}
		T->OnTargetHit.AddDynamic(this, &USwitchingSubtest::OnSwitchHit);
		T->SetHighlighted(false);
		Targets.Add(T);
	}
}

void USwitchingSubtest::OnSubtestEnd()
{
	Super::OnSubtestEnd();

	if (AAssessmentPawn* AP = Cast<AAssessmentPawn>(PlayerPawn))
	{
		AP->OnFire.RemoveAll(this);
	}

	if (SpawnManager)
	{
		for (ATarget* T : Targets)
		{
			if (T) { SpawnManager->DestroyTarget(T); }
		}
	}
	Targets.Empty();
}

void USwitchingSubtest::OnTrialStart()
{
	if (Targets.Num() < 2) return;

	CurrentRoundResult.TrialIndex = RoundResults.Num();

	// New active target: uniformly random among all EXCEPT the current one, so it always
	// switches. (For N == 2 this degenerates to deterministic back-and-forth.)
	PreviousActiveIndex = ActiveIndex;
	if (ActiveIndex == INDEX_NONE)
	{
		ActiveIndex = RandomStream.RandRange(0, Targets.Num() - 1);
	}
	else
	{
		int32 Pick = RandomStream.RandRange(0, Targets.Num() - 2);
		if (Pick >= ActiveIndex) { ++Pick; }   // skip the current index -> uniform over the others
		ActiveIndex = Pick;
	}

	for (int32 i = 0; i < Targets.Num(); ++i)
	{
		if (Targets[i]) { Targets[i]->SetHighlighted(i == ActiveIndex); }
	}

	CurrentRoundResult.ActiveIndex = ActiveIndex;
	CurrentRoundResult.ActiveLocalPos = Targets[ActiveIndex]->GetRootComponent()->GetRelativeLocation();
	CurrentRoundResult.SwitchAngleDeg = (PreviousActiveIndex != INDEX_NONE)
		? AngleBetweenTargetsDeg(PreviousActiveIndex, ActiveIndex)
		: 0.f;

	bSwitchActive = true;
	Stopwatch.Start();
	SetTrialTimer(SwitchingSubtestConfig.SwitchTimeoutSeconds);   // timeout -> EndTrial (Outcome stays Miss)
}

void USwitchingSubtest::OnSwitchFire()
{
	// Only record shots while a switch is in progress.
	if (!bSwitchActive || !PlayerPawn || !Targets.IsValidIndex(ActiveIndex)) return;

	const ATarget* Active = Targets[ActiveIndex];
	if (!Active) return;

	const AController* Controller = PlayerPawn->GetController();
	if (!Controller) return;

	FVector ViewLocation;
	FRotator ViewRotation;
	Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);   // matches the shoot trace

	const FVector ToTarget = (Active->GetActorLocation() - ViewLocation).GetSafeNormal();
	const FVector Local = ViewRotation.UnrotateVector(ToTarget);   // X = forward, Y = right, Z = up

	FSwitchShot Shot;
	Shot.ShotIndex = CurrentRoundResult.Shots.Num();
	Shot.YawErrorDeg = FMath::RadiansToDegrees(FMath::Atan2(Local.Y, Local.X));     // + = target right of aim
	Shot.PitchErrorDeg = FMath::RadiansToDegrees(FMath::Atan2(Local.Z, Local.X));   // + = target above aim
	Shot.TimeMs = FMath::RoundToInt(Stopwatch.ElapsedSeconds() * 1000.0);

	CurrentRoundResult.Shots.Add(Shot);   // bHit/bWrongTarget set by OnSwitchHit if this shot connected
}

void USwitchingSubtest::OnSwitchHit(ATarget* HitTarget)
{
	if (!bSwitchActive) return;

	if (Targets.IsValidIndex(ActiveIndex) && HitTarget == Targets[ActiveIndex])
	{
		// OnSwitchFire ran first this shot, so the last recorded shot is the one that connected.
		if (CurrentRoundResult.Shots.Num() > 0) { CurrentRoundResult.Shots.Last().bHit = true; }
		CurrentRoundResult.Outcome = ESwitchOutcome::Hit;
		EndTrial();   // advances; next switch starts back-to-back
	}
	else
	{
		// Mis-switch: a non-active target was hit. Flag the shot, but don't advance.
		if (CurrentRoundResult.Shots.Num() > 0) { CurrentRoundResult.Shots.Last().bWrongTarget = true; }
	}
}

void USwitchingSubtest::OnTrialEnd()
{
	Super::OnTrialEnd();

	bSwitchActive = false;

	// Switch over (hit or timeout): drop the active target's highlight immediately,
	// so it doesn't linger before the next target lights up.
	if (Targets.IsValidIndex(ActiveIndex) && Targets[ActiveIndex])
	{
		Targets[ActiveIndex]->SetHighlighted(false);
	}

	RoundResults.Add(CurrentRoundResult);

	CurrentRoundResult = FSwitchingRoundResult();
	Stopwatch.Stop();
	Stopwatch.Reset();
	// ActiveIndex is intentionally kept so the next switch measures the flick from it.
}

float USwitchingSubtest::AngleBetweenTargetsDeg(int32 IndexA, int32 IndexB) const
{
	if (!PlayerPawn || !Targets.IsValidIndex(IndexA) || !Targets.IsValidIndex(IndexB)) return 0.f;
	if (!Targets[IndexA] || !Targets[IndexB]) return 0.f;

	const AController* Controller = PlayerPawn->GetController();
	if (!Controller) return 0.f;

	FVector ViewLocation;
	FRotator ViewRotation;
	Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);

	const FVector DirA = (Targets[IndexA]->GetActorLocation() - ViewLocation).GetSafeNormal();
	const FVector DirB = (Targets[IndexB]->GetActorLocation() - ViewLocation).GetSafeNormal();
	return FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(DirA, DirB), -1.f, 1.f)));
}

FName USwitchingSubtest::GetSubtestId() const
{
	return FName(TEXT("Switching"));
}

TArray<FString> USwitchingSubtest::GetSubtestTrialRecordsJson()
{
	TArray<FString> Out;
	for (const FSwitchingRoundResult& R : RoundResults)
	{
		FString Json;
		FJsonObjectConverter::UStructToJsonObjectString(R, Json);
		Out.Add(Json);
	}
	return Out;
}

FString USwitchingSubtest::GetSubtestAggregateJson()
{
	// Minimal — real metrics are derived downstream from the per-shot data.
	FSwitchingAggregate Agg;
	for (const FSwitchingRoundResult& R : RoundResults)
	{
		if (R.Outcome == ESwitchOutcome::Hit) { Agg.HitCount++; }
		else { Agg.MissCount++; }
		Agg.TotalShots += R.Shots.Num();
	}

	FString Json;
	FJsonObjectConverter::UStructToJsonObjectString(Agg, Json);
	return Json;
}
