// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnManager.generated.h"

class ATarget;

UCLASS()
class RETICLE_API ASpawnManager : public AActor
{
	GENERATED_BODY()

public:
	ASpawnManager();

	UFUNCTION(BlueprintCallable)
	void OnLevelStart(int32 NumOfTargets, float SetTargetShowTime, float SetTargetHideTime);

protected:
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<ATarget> TargetClass;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	int32 PoolSize = 8;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	int32 MaxActiveTargets = 1;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	FVector2D SpawnOffset = FVector2D(200, 200);

	UPROPERTY(EditAnywhere, Category = "Spawning")
	float MinSeparation = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	bool bUseSeed = false;

	UPROPERTY(EditAnywhere, Category = "Spawning", meta = (EditCondition = "bUseSeed"))
	int32 Seed = 0;

private:
	UPROPERTY()
	TArray<ATarget*> TargetPool;

	int32 TargetsRemaining = 0;
	int32 TotalHits = 0;
	int32 TotalMisses = 0;

	float TargetShowTime = 1.0f;
	float TargetHideTime = 0.5f;

	FRandomStream RandomStream;

	void InitializePool();
	void EnsureActiveTargets();
	void ScheduleEnsureActive();
	void OnLevelComplete();

	int32 CountActiveTargets() const;
	ATarget* FindInactiveTarget() const;

	TArray<FVector> GenerateSpawnPositions(int32 NumOfTargets);
	FVector GenerateOnePosition();
	bool IsTooCloseToActiveTarget(const FVector& Pos) const;

	UFUNCTION()
	void HandleTargetHit(ATarget* HitTarget, bool bHeadshot);

	UFUNCTION()
	void HandleTargetExpired(ATarget* ExpiredTarget);
};