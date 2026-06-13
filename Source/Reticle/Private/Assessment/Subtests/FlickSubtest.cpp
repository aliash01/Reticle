// Fill out your copyright notice in the Description page of Project Settings.

#include "Assessment/Subtests/FlickSubtest.h"

#include "JsonObjectConverter.h"
#include "Assessment/AssessmentLog.h"
#include "Assessment/AssessmentPawn.h"
#include "Assessment/SubtestConfigs/FlickConfig.h"
#include "Common/SpawnManager.h"
#include "Common/Target/Target.h"
#include "GameFramework/Controller.h"

void UFlickSubtest::Initialise(ASpawnManager* InSpawnManager, APawn* InPlayerPawn)
{
	Super::Initialise(InSpawnManager, InPlayerPawn);

	// Bind fire to capture every shot (hit or miss). No LockLook — flick needs free aim.
	if (AAssessmentPawn* AP = Cast<AAssessmentPawn>(InPlayerPawn))
	{
		AP->OnFire.AddUObject(this, &UFlickSubtest::OnFlickFire);
	}

	if (SpawnManager)
	{
		Target = SpawnManager->SpawnTarget(FVector::ZeroVector, 0.f);

		if (!Target)
		{
			UE_LOG(LogAssessment, Error, TEXT("Failed to spawn target"));
			return;
		}

		Target->Deactivate();
		Target->OnTargetExpired.AddDynamic(this, &UFlickSubtest::OnFlickMissed);
		Target->OnTargetHit.AddDynamic(this, &UFlickSubtest::OnFlickHit);
	}
}

void UFlickSubtest::OnSubtestStart(USubtestConfigBase* Config)
{
	Super::OnSubtestStart(Config);

	UFlickConfig* FlickCfg = Cast<UFlickConfig>(Config);

	if (!FlickCfg)
	{
		UE_LOG(LogAssessment, Error, TEXT("FlickConfig cast failed"));
		EndSubtest(true);   // don't run a misconfigured subtest
		return;
	}

	FlickSubtestConfig = FlickCfg->GetFlickConfig();
	ResponseWindowSeconds = FlickSubtestConfig.ResponseWindowSeconds;
}

void UFlickSubtest::OnSubtestEnd()
{
	Super::OnSubtestEnd();

	if (AAssessmentPawn* AP = Cast<AAssessmentPawn>(PlayerPawn))
	{
		AP->OnFire.RemoveAll(this);
	}

	if (SpawnManager)
	{
		SpawnManager->DestroyTarget(Target);
	}
}

void UFlickSubtest::OnTrialStart()
{
	if (!Target) return;

	// Random local point inside the spawn area; store it as the trial's flick context.
	CurrentRoundResult.TrialIndex = RoundResults.Num();   // 0-based index this trial will occupy

	const FVector LocalPosition = SpawnManager->GeneratePosition(RandomStream);
	CurrentRoundResult.TargetLocalPos = LocalPosition;

	Target->Activate(LocalPosition, ResponseWindowSeconds);   // lifetime = response window -> expiry is a miss
	Stopwatch.Start();
}

void UFlickSubtest::OnFlickFire()
{
	// Only record shots while the target is up (ignore between-trials clicks).
	if (!Target || !Target->IsActivated() || !PlayerPawn) return;

	const AController* Controller = PlayerPawn->GetController();
	if (!Controller) return;

	FVector ViewLocation;
	FRotator ViewRotation;
	Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);   // matches the shoot trace

	const FVector ToTarget = (Target->GetActorLocation() - ViewLocation).GetSafeNormal();
	const FVector Local = ViewRotation.UnrotateVector(ToTarget);   // X = forward, Y = right, Z = up

	FFlickShot Shot;
	Shot.ShotIndex = CurrentRoundResult.Shots.Num();   // 0-based order within this trial
	Shot.YawErrorDeg = FMath::RadiansToDegrees(FMath::Atan2(Local.Y, Local.X));     // + = target right of aim
	Shot.PitchErrorDeg = FMath::RadiansToDegrees(FMath::Atan2(Local.Z, Local.X));   // + = target above aim
	Shot.TimeMs = FMath::RoundToInt(Stopwatch.ElapsedSeconds() * 1000.0);
	Shot.bHit = false;   // set true by OnFlickHit if this shot connected

	CurrentRoundResult.Shots.Add(Shot);
}

void UFlickSubtest::OnFlickHit(ATarget* HitTarget, bool bHeadshot)
{
	HitTarget->Deactivate();

	// OnFlickFire ran first this shot, so the last recorded shot is the one that connected.
	if (CurrentRoundResult.Shots.Num() > 0)
	{
		CurrentRoundResult.Shots.Last().bHit = true;
	}
	CurrentRoundResult.Outcome = EFlickOutcome::Hit;

	EndTrial();
}

void UFlickSubtest::OnFlickMissed(ATarget* ExpiredTarget)
{
	ExpiredTarget->Deactivate();

	CurrentRoundResult.Outcome = EFlickOutcome::Miss;

	EndTrial();
}

void UFlickSubtest::OnTrialEnd()
{
	Super::OnTrialEnd();

	RoundResults.Add(CurrentRoundResult);

	CurrentRoundResult = FFlickRoundResult();
	Stopwatch.Stop();
	Stopwatch.Reset();
}

FName UFlickSubtest::GetSubtestId() const
{
	return FName(TEXT("Flick"));
}

TArray<FString> UFlickSubtest::GetSubtestTrialRecordsJson()
{
	TArray<FString> Out;
	for (const FFlickRoundResult& R : RoundResults)
	{
		FString Json;
		FJsonObjectConverter::UStructToJsonObjectString(R, Json);
		Out.Add(Json);
	}
	return Out;
}

FString UFlickSubtest::GetSubtestAggregateJson()
{
	// Minimal — real metrics are derived downstream from the per-shot data.
	FFlickAggregate Agg;
	for (const FFlickRoundResult& R : RoundResults)
	{
		if (R.Outcome == EFlickOutcome::Hit) { Agg.HitCount++; }
		else { Agg.MissCount++; }
		Agg.TotalShots += R.Shots.Num();
	}

	FString Json;
	FJsonObjectConverter::UStructToJsonObjectString(Agg, Json);
	return Json;
}
