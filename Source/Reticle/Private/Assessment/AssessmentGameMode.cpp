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
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

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
	//StartPrecisionSubtest();
	StartAssessment();
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

void AAssessmentGameMode::StartAssessment()
{
	if (Battery.Num() == 0)
	{
		UE_LOG(LogAssessment, Error, TEXT("StartAssessment: Battery is empty — assign subtests in the editor"));
		return;
	}

	BatterySessionId = FGuid::NewGuid();
	SessionStartedUtc = FDateTime::UtcNow();
	SessionResults.Reset();
	CurrentStepIndex = 0;

	UE_LOG(LogAssessment, Log, TEXT("Assessment started — session=%s, %d subtests"),
		*BatterySessionId.ToString(), Battery.Num());

	RunStep(0);
}

void AAssessmentGameMode::RunStep(int32 Index)
{
	if (!Battery.IsValidIndex(Index))
	{
		FinishAssessment();
		return;
	}

	const FBatteryStep& Step = Battery[Index];
	if (!Step.SubtestClass || !Step.Config)
	{
		UE_LOG(LogAssessment, Error, TEXT("Battery step %d missing class or config — skipping"), Index);
		RunStep(Index + 1);
		return;
	}

	ActiveSubtest = NewObject<USubtestBase>(this, Step.SubtestClass);
	if (!ActiveSubtest)
	{
		UE_LOG(LogAssessment, Error, TEXT("Battery step %d: failed to create subtest"), Index);
		RunStep(Index + 1);
		return;
	}

	ActiveSubtest->OnSubtestEnded.AddUObject(this, &AAssessmentGameMode::HandleSubtestEnded);
	ActiveSubtest->Initialise(ActiveSpawnManager, UGameplayStatics::GetPlayerPawn(this, 0));

	// Every subtest in the battery shares the one session id.
	Step.Config->GetConfig().SessionId = BatterySessionId;
	ActiveSubtest->BeginSubtest(Step.Config);
}

void AAssessmentGameMode::HandleSubtestEnded(const FSubtestResult& Result)
{
	UE_LOG(LogAssessment, Log, TEXT("Subtest %s ended (%d trials, aborted=%d)"),
		*Result.SubtestId.ToString(), Result.TrialsCompleted, Result.bAborted);

	SessionResults.Add(Result);

	RunStep(++CurrentStepIndex);   // next subtest, or FinishAssessment when out of range
}

void AAssessmentGameMode::FinishAssessment()
{
	FAssessmentSession Session;
	Session.SessionId = BatterySessionId;
	Session.StartedAtUtc = SessionStartedUtc;
	Session.CompletedAtUtc = FDateTime::UtcNow();
	Session.Results = SessionResults;

	FString Json;
	FJsonObjectConverter::UStructToJsonObjectString(Session, Json);

	const FString Dir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("AssessmentSessions"));
	IFileManager::Get().MakeDirectory(*Dir, /*Tree=*/true);
	const FString Path = FPaths::Combine(Dir, BatterySessionId.ToString() + TEXT(".json"));

	if (FFileHelper::SaveStringToFile(Json, *Path))
	{
		UE_LOG(LogAssessment, Log, TEXT("Assessment complete — %d subtests saved to %s"),
			SessionResults.Num(), *Path);
	}
	else
	{
		UE_LOG(LogAssessment, Error, TEXT("Assessment complete but failed to write %s"), *Path);
	}
}