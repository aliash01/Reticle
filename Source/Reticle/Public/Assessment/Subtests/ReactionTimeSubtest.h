// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Assessment/Subtests/SubtestBase.h"
#include "Common/Target/Target.h"
#include "ReactionTimeSubtest.generated.h"

UENUM(BlueprintType)
enum class EReactionOutcome : uint8
{
	Valid,
	NoResponse,
	Invalidated
};

USTRUCT(BlueprintType)
struct FReactionTimeRoundResult
{
	GENERATED_BODY()

	UPROPERTY() int32 ForePeriodMs;
	UPROPERTY() int32 ReactionTimeMs = -1;
	UPROPERTY() int32 NumFalseStarts = 0;
	UPROPERTY() EReactionOutcome Outcome;
};

USTRUCT(BlueprintType)
struct FReactionTimeAggregate
{
	GENERATED_BODY()

	UPROPERTY() float MeanRtMs = 0.f;
	UPROPERTY() float SdRtMs = 0.f;
	UPROPERTY() int32 ValidCount = 0;
	UPROPERTY() int32 NoResponseCount = 0;
	UPROPERTY() int32 TotalFalseStarts = 0;
};

/**
 * 
*/ 
UCLASS()
class RETICLE_API UReactionTimeSubtest : public USubtestBase
{
	GENERATED_BODY()
	
	virtual void OnTrialStart() override;
	void OnFalseStart();
	virtual void Initialise(ASpawnManager* InSpawnManager, APawn* InPlayerPawn) override;
	void OnFire();
	void ShowTarget();

	UFUNCTION()
	void OnTrialTimeExpired(ATarget* ExpiredTarget);
	UFUNCTION()
	void OnTrialCompleted(ATarget* HitTarget, bool bHeadshot);
	virtual void OnTrialEnd() override;
	virtual FName GetSubtestId() const override;
	virtual void OnSubtestStart(USubtestConfigBase* Config) override;
	virtual void OnSubtestEnd() override;

	virtual TArray<FString> GetSubtestTrialRecordsJson() override;
	virtual FString GetSubtestAggregateJson() override;

	UPROPERTY()
	ATarget* Target;

	int32 FalseStartsCount = 0;
	int32 FalseStartsCap;
	
	bool bInForeperiod = false;
	float TargetLifetime = 2.f;
	float ForeperiodMinTime = 1.f;
	float ForeperiodMaxTime = 3.f;
	
	FTimerHandle ForeperiodTimer;

	FReactionTimeRoundResult CurrentRoundResult;
	TArray<FReactionTimeRoundResult> RoundResults;
};
