// Fill out your copyright notice in the Description page of Project Settings.


#include "Assessment/AssessmentGameMode.h"
#include "Assessment/AssessmentLog.h"
#include "Assessment/SubtestConfigs/SubtestConfigBase.h"
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

	StartAssessment();
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

	USubtestConfigBase* Config = Battery[Index];
	if (!Config)
	{
		UE_LOG(LogAssessment, Error, TEXT("Battery step %d has no config — skipping"), Index);
		RunStep(Index + 1);
		return;
	}

	const TSubclassOf<USubtestBase> SubtestClass = Config->GetSubtestClass();
	if (!SubtestClass)
	{
		UE_LOG(LogAssessment, Error, TEXT("Battery step %d: config %s declares no subtest class — skipping"),
			Index, *Config->GetName());
		RunStep(Index + 1);
		return;
	}

	ActiveSubtest = NewObject<USubtestBase>(this, SubtestClass);
	if (!ActiveSubtest)
	{
		UE_LOG(LogAssessment, Error, TEXT("Battery step %d: failed to create subtest"), Index);
		RunStep(Index + 1);
		return;
	}

	ActiveSubtest->OnSubtestEnded.AddUObject(this, &AAssessmentGameMode::HandleSubtestEnded);
	ActiveSubtest->Initialise(ActiveSpawnManager, UGameplayStatics::GetPlayerPawn(this, 0));

	// Every subtest in the battery shares the one session id.
	Config->GetConfig().SessionId = BatterySessionId;
	ActiveSubtest->BeginSubtest(Config);
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