// Fill out your copyright notice in the Description page of Project Settings.


#include "Assessment/AssessmentGameMode.h"
#include "Assessment/AssessmentLog.h"
#include "Assessment/SubtestConfigs/FlickConfig.h"
#include "Assessment/SubtestConfigs/ReactionTimeConfig.h"
#include "Assessment/Subtests/FlickSubtest.h"
#include "Assessment/Subtests/ReactionTimeSubtest.h"
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
	StartFlickSubtest();
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

void AAssessmentGameMode::HandleSubtestEnded(const FSubtestResult& Result)
{
	UE_LOG(LogAssessment, Log, TEXT("%s ended — aggregate: %s"),
		*Result.SubtestId.ToString(), *Result.AggregateJson);

	for (const FString& Trial : Result.TrialRecordsJson)
	{
		UE_LOG(LogAssessment, Log, TEXT("  trial: %s"), *Trial);
	}
}