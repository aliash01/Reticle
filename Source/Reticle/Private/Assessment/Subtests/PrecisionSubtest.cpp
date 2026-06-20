// Fill out your copyright notice in the Description page of Project Settings.

#include "Assessment/Subtests/PrecisionSubtest.h"

#include "JsonObjectConverter.h"
#include "Assessment/AssessmentLog.h"
#include "Assessment/AssessmentPawn.h"
#include "Assessment/SubtestConfigs/PrecisionConfig.h"
#include "Common/SpawnManager.h"
#include "Common/Target/Target.h"
#include "GameFramework/Controller.h"

void UPrecisionSubtest::Initialise(ASpawnManager* InSpawnManager, APawn* InPlayerPawn)
{
	Super::Initialise(InSpawnManager, InPlayerPawn);

	// Capture every shot (hit or miss), like Flick. Semi-auto fire (pawn default).
	if (AAssessmentPawn* AP = Cast<AAssessmentPawn>(InPlayerPawn))
	{
		AP->OnFire.AddUObject(this, &UPrecisionSubtest::OnPrecisionFire);
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
		Target->OnTargetExpired.AddDynamic(this, &UPrecisionSubtest::OnPrecisionMissed);
		Target->OnTargetHit.AddDynamic(this, &UPrecisionSubtest::OnPrecisionHit);
	}
}

void UPrecisionSubtest::OnSubtestStart(USubtestConfigBase* Config)
{
	Super::OnSubtestStart(Config);

	UPrecisionConfig* PrecisionCfg = Cast<UPrecisionConfig>(Config);

	if (!PrecisionCfg)
	{
		UE_LOG(LogAssessment, Error, TEXT("PrecisionConfig cast failed"));
		EndSubtest(true);   // don't run a misconfigured subtest
		return;
	}

	PrecisionSubtestConfig = PrecisionCfg->GetPrecisionConfig();
}

void UPrecisionSubtest::OnSubtestEnd()
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

void UPrecisionSubtest::OnTrialStart()
{
	if (!Target) return;

	CurrentRoundResult.TrialIndex = RoundResults.Num();

	// Fixed distance, random direction (no distance variation); randomised size each trial.
	const float Angle = RandomStream.FRandRange(0.f, 2.f * PI);
	const float R = PrecisionSubtestConfig.Distance;
	const FVector LocalPosition(0.f, R * FMath::Cos(Angle), R * FMath::Sin(Angle));
	const float SizeScale = RandomStream.FRandRange(
		PrecisionSubtestConfig.MinSizeScale, PrecisionSubtestConfig.MaxSizeScale);

	CurrentRoundResult.TargetLocalPos = LocalPosition;
	CurrentRoundResult.TargetSizeScale = SizeScale;

	Target->SetSize(SizeScale);
	Target->Activate(LocalPosition, PrecisionSubtestConfig.ResponseWindowSeconds);   // lifetime = window -> expiry is a miss
	Stopwatch.Start();
}

void UPrecisionSubtest::OnPrecisionFire()
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

	FPrecisionShot Shot;
	Shot.ShotIndex = CurrentRoundResult.Shots.Num();
	Shot.YawErrorDeg = FMath::RadiansToDegrees(FMath::Atan2(Local.Y, Local.X));     // + = target right of aim
	Shot.PitchErrorDeg = FMath::RadiansToDegrees(FMath::Atan2(Local.Z, Local.X));   // + = target above aim
	Shot.TimeMs = FMath::RoundToInt(Stopwatch.ElapsedSeconds() * 1000.0);

	CurrentRoundResult.Shots.Add(Shot);
}

void UPrecisionSubtest::OnPrecisionHit(ATarget* HitTarget)
{
	HitTarget->Deactivate();

	// OnPrecisionFire ran first this shot, so the last recorded shot is the one that connected.
	if (CurrentRoundResult.Shots.Num() > 0) { CurrentRoundResult.Shots.Last().bHit = true; }
	CurrentRoundResult.Outcome = EPrecisionOutcome::Hit;

	EndTrial();
}

void UPrecisionSubtest::OnPrecisionMissed(ATarget* ExpiredTarget)
{
	ExpiredTarget->Deactivate();

	CurrentRoundResult.Outcome = EPrecisionOutcome::Miss;

	EndTrial();
}

void UPrecisionSubtest::OnTrialEnd()
{
	Super::OnTrialEnd();

	RoundResults.Add(CurrentRoundResult);

	CurrentRoundResult = FPrecisionRoundResult();
	Stopwatch.Stop();
	Stopwatch.Reset();
}

FName UPrecisionSubtest::GetSubtestId() const
{
	return FName(TEXT("Precision"));
}

TArray<FString> UPrecisionSubtest::GetSubtestTrialRecordsJson()
{
	TArray<FString> Out;
	for (const FPrecisionRoundResult& R : RoundResults)
	{
		FString Json;
		FJsonObjectConverter::UStructToJsonObjectString(R, Json);
		Out.Add(Json);
	}
	return Out;
}

FString UPrecisionSubtest::GetSubtestAggregateJson()
{
	// Minimal — real metrics are derived downstream from the per-shot data + per-trial size.
	FPrecisionAggregate Agg;
	for (const FPrecisionRoundResult& R : RoundResults)
	{
		if (R.Outcome == EPrecisionOutcome::Hit) { Agg.HitCount++; }
		else { Agg.MissCount++; }
		Agg.TotalShots += R.Shots.Num();
	}

	FString Json;
	FJsonObjectConverter::UStructToJsonObjectString(Agg, Json);
	return Json;
}
