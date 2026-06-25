// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Subtests/SubtestBase.h"
#include "AssessmentGameMode.generated.h"

class USubtestBase;
class ASpawnManager;

// One completed assessment run — the shared session id plus every subtest's result.
// Serialized to JSON on disk as the handoff artifact for the analysis pipeline.
USTRUCT()
struct FAssessmentSession
{
	GENERATED_BODY()

	UPROPERTY() FString SchemaVersion = TEXT("1.0");
	UPROPERTY() FGuid SessionId;
	UPROPERTY() FDateTime StartedAtUtc;
	UPROPERTY() FDateTime CompletedAtUtc;
	UPROPERTY() TArray<FSubtestResult> Results;
};

/**
 *
 */
UCLASS()
class RETICLE_API AAssessmentGameMode : public AGameModeBase
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<ASpawnManager> ActiveSpawnManager;

public:
	void RegisterActiveSpawnManager(ASpawnManager* SpawnManager);
	virtual void BeginPlay() override;

	// Runs every subtest in Battery back-to-back under one SessionId, then persists the session.
	void StartAssessment();

	void HandleSubtestEnded(const FSubtestResult& Result);

	// The ordered battery — fill in the editor with config assets; each config knows which
	// subtest class runs it (USubtestConfigBase::GetSubtestClass), so there's nothing else to pair.
	UPROPERTY(EditAnywhere, Category="Assessment")
	TArray<TObjectPtr<USubtestConfigBase>> Battery;


private:
	void RunStep(int32 Index);
	void FinishAssessment();

	UPROPERTY() USubtestBase* ActiveSubtest = nullptr;

	// Runtime state for the active battery run.
	FGuid BatterySessionId;
	int32 CurrentStepIndex = 0;
	FDateTime SessionStartedUtc;
	TArray<FSubtestResult> SessionResults;
};
