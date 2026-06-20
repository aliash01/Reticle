// Fill out your copyright notice in the Description page of Project Settings.


#include "Assessment/AssessmentGameMode.h"
#include "Assessment/AssessmentLog.h"
#include "Assessment/SubtestConfigs/FlickConfig.h"
#include "Assessment/SubtestConfigs/ReactionTimeConfig.h"
#include "Assessment/SubtestConfigs/TrackingConfig.h"
#include "Assessment/Subtests/FlickSubtest.h"
#include "Assessment/Subtests/ReactionTimeSubtest.h"
#include "Assessment/Subtests/TrackingSubtest.h"
#include "Assessment/SubtestConfigs/SwitchingConfig.h"
#include "Assessment/Subtests/SwitchingSubtest.h"
#include "Assessment/SubtestConfigs/PrecisionConfig.h"
#include "Assessment/Subtests/PrecisionSubtest.h"
#include "Assessment/Subtests/SubtestBase.h"
#include "Common/SpawnManager.h"
#include "Kismet/GameplayStatics.h"

void AAssessmentGameMode::RegisterActiveSpawnManager(ASpawnManager* SpawnManager)
{
	ActiveSpawnManager = SpawnManager;
}

void AAssessmentGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!ActiveSpawnManager)
	{
		UE_LOG(LogAssessment, Warning, TEXT("SpawnManager is null. Potentially not existing in level."))
	}

	//StartReactionTimeSubtest();
	//StartFlickSubtest();
	//StartTrackingSubtest();
	//StartSwitchingSubtest();
	//StartReactiveTrackingSubtest();
	StartPrecisionSubtest();
}

void AAssessmentGameMode::StartReactionTimeSubtest()
{
	ActiveSubtest = NewObject<UReactionTimeSubtest>(this, TEXT("SubtestBase"));
	if (!ActiveSubtest)
	{
		UE_LOG(LogAssessment, Error, TEXT("Failed to create SubtestBase"));
		return;
	}

	if (!ReactionTimeConfig)
	{
		UE_LOG(LogAssessment, Error, TEXT("ReactionTimeConfig not assigned"));
		return;
	}

	ActiveSubtest->OnSubtestEnded.AddUObject(this, &AAssessmentGameMode::HandleSubtestEnded);
	ActiveSubtest->Initialise(ActiveSpawnManager, UGameplayStatics::GetPlayerPawn(this, 0));
	
	ReactionTimeConfig->GetConfig().SessionId = FGuid::NewGuid();
	ActiveSubtest->BeginSubtest(ReactionTimeConfig);
}

void AAssessmentGameMode::StartFlickSubtest()
{
	ActiveSubtest = NewObject<UFlickSubtest>(this);
	if (!ActiveSubtest)
	{
		UE_LOG(LogAssessment, Error, TEXT("Failed to create FlickSubtest"));
		return;
	}

	if (!FlickConfig)
	{
		UE_LOG(LogAssessment, Error, TEXT("FlickConfig not assigned"));
		return;
	}

	ActiveSubtest->OnSubtestEnded.AddUObject(this, &AAssessmentGameMode::HandleSubtestEnded);
	ActiveSubtest->Initialise(ActiveSpawnManager, UGameplayStatics::GetPlayerPawn(this, 0));

	FlickConfig->GetConfig().SessionId = FGuid::NewGuid();
	ActiveSubtest->BeginSubtest(FlickConfig);
}

void AAssessmentGameMode::StartTrackingSubtest()
{
	ActiveSubtest = NewObject<UTrackingSubtest>(this);
	if (!ActiveSubtest)
	{
		UE_LOG(LogAssessment, Error, TEXT("Failed to create TrackingSubtest"));
		return;
	}

	if (!TrackingConfig)
	{
		UE_LOG(LogAssessment, Error, TEXT("TrackingConfig not assigned"));
		return;
	}

	ActiveSubtest->OnSubtestEnded.AddUObject(this, &AAssessmentGameMode::HandleSubtestEnded);
	ActiveSubtest->Initialise(ActiveSpawnManager, UGameplayStatics::GetPlayerPawn(this, 0));

	TrackingConfig->GetConfig().SessionId = FGuid::NewGuid();
	ActiveSubtest->BeginSubtest(TrackingConfig);
}

void AAssessmentGameMode::StartSwitchingSubtest()
{
	ActiveSubtest = NewObject<USwitchingSubtest>(this);
	if (!ActiveSubtest)
	{
		UE_LOG(LogAssessment, Error, TEXT("Failed to create SwitchingSubtest"));
		return;
	}

	if (!SwitchingConfig)
	{
		UE_LOG(LogAssessment, Error, TEXT("SwitchingConfig not assigned"));
		return;
	}

	ActiveSubtest->OnSubtestEnded.AddUObject(this, &AAssessmentGameMode::HandleSubtestEnded);
	ActiveSubtest->Initialise(ActiveSpawnManager, UGameplayStatics::GetPlayerPawn(this, 0));

	SwitchingConfig->GetConfig().SessionId = FGuid::NewGuid();
	ActiveSubtest->BeginSubtest(SwitchingConfig);
}

void AAssessmentGameMode::StartReactiveTrackingSubtest()
{
	// Same class/config type as Smooth Tracking; the asset's PathMode = Reactive selects the
	// erratic path and makes GetSubtestId report "ReactiveTracking".
	ActiveSubtest = NewObject<UTrackingSubtest>(this);
	if (!ActiveSubtest)
	{
		UE_LOG(LogAssessment, Error, TEXT("Failed to create ReactiveTrackingSubtest"));
		return;
	}

	if (!ReactiveTrackingConfig)
	{
		UE_LOG(LogAssessment, Error, TEXT("ReactiveTrackingConfig not assigned"));
		return;
	}

	ActiveSubtest->OnSubtestEnded.AddUObject(this, &AAssessmentGameMode::HandleSubtestEnded);
	ActiveSubtest->Initialise(ActiveSpawnManager, UGameplayStatics::GetPlayerPawn(this, 0));

	ReactiveTrackingConfig->GetConfig().SessionId = FGuid::NewGuid();
	ActiveSubtest->BeginSubtest(ReactiveTrackingConfig);
}

void AAssessmentGameMode::StartPrecisionSubtest()
{
	ActiveSubtest = NewObject<UPrecisionSubtest>(this);
	if (!ActiveSubtest)
	{
		UE_LOG(LogAssessment, Error, TEXT("Failed to create PrecisionSubtest"));
		return;
	}

	if (!PrecisionConfig)
	{
		UE_LOG(LogAssessment, Error, TEXT("PrecisionConfig not assigned"));
		return;
	}

	ActiveSubtest->OnSubtestEnded.AddUObject(this, &AAssessmentGameMode::HandleSubtestEnded);
	ActiveSubtest->Initialise(ActiveSpawnManager, UGameplayStatics::GetPlayerPawn(this, 0));

	PrecisionConfig->GetConfig().SessionId = FGuid::NewGuid();
	ActiveSubtest->BeginSubtest(PrecisionConfig);
}

void AAssessmentGameMode::HandleSubtestEnded(const FSubtestResult& Result)
{
	UE_LOG(LogAssessment, Log, TEXT("%s ended — aggregate: %s"),
		*Result.SubtestId.ToString(), *Result.AggregateJson);

	for (const FString& Trial : Result.TrialRecordsJson)
	{
		UE_LOG(LogAssessment, Log, TEXT("  trial: %s"), *Trial);
	}
}