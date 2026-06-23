// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Subtests/SubtestBase.h"
#include "AssessmentGameMode.generated.h"

class UReactionTimeConfig;
class UFlickConfig;
class UTrackingConfig;
class USwitchingConfig;
class UPrecisionConfig;
class USubtestBase;
class ASpawnManager;

// One entry in the assessment battery: which subtest class to run and the config asset to run
// it with. Paired in the editor; the order of the Battery array is the order subtests run.
USTRUCT(BlueprintType)
struct FBatteryStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) TSubclassOf<USubtestBase> SubtestClass;
	UPROPERTY(EditAnywhere) USubtestConfigBase* Config;
};

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

	void StartReactionTimeSubtest();
	void StartFlickSubtest();
	void StartTrackingSubtest();
	void StartSwitchingSubtest();
	void StartReactiveTrackingSubtest();
	void StartPrecisionSubtest();
	void HandleSubtestEnded(const FSubtestResult& Result);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Subtest Config")
	UReactionTimeConfig* ReactionTimeConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Subtest Config")
	UFlickConfig* FlickConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Subtest Config")
	UTrackingConfig* TrackingConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Subtest Config")
	USwitchingConfig* SwitchingConfig;

	// Same UTrackingConfig type as TrackingConfig — set its PathMode to Reactive in the asset.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Subtest Config")
	UTrackingConfig* ReactiveTrackingConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Subtest Config")
	UPrecisionConfig* PrecisionConfig;

	// The ordered battery — fill in the editor with (subtest class, config asset) pairs.
	UPROPERTY(EditAnywhere, Category="Assessment")
	TArray<FBatteryStep> Battery;


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
