// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnManager.generated.h"

class ATarget;
class UBoxComponent;

// Relayed up to the owning class (e.g. the active subtest) when one of this
// manager's targets is hit, or expires unhit. The owner binds these once.
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTargetHitSignature, ATarget* /*Target*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTargetMissedSignature, ATarget* /*Target*/);

/**
 * Thin target-spawning service. Spawns/destroys targets on request and relays
 * each target's hit/expiry events up to its owner. Owns no loop or scoring —
 * the owning subtest decides what/where to spawn and what a hit means.
 */
UCLASS()
class RETICLE_API ASpawnManager : public AActor
{
	GENERATED_BODY()

public:
	ASpawnManager();
	virtual void BeginPlay() override;

	// Spawn a target at LocalLocation (relative to this manager; (0,0,0) == its
	// centre) and track it. LifeTime <= 0 means no auto-expiry — the owner
	// controls when it ends. Returns the spawned target.
	ATarget* SpawnTarget(const FVector& LocalLocation, float LifeTime = 0.f);

	// Destroy a target this manager spawned and stop tracking it.
	void DestroyTarget(ATarget* Target);

	// Pick a random point (local to this manager, pass straight to SpawnTarget)
	// that isn't within MinSeparation of an active target. Caller supplies the
	// stream so randomness stays seed-driven.
	FVector GeneratePosition(FRandomStream& Stream) const;

	// True if Location is within MinSeparation of any currently-active target.
	bool IsTooCloseToActiveTarget(const FVector& Location) const;

	// Owner (subtest) subscribes once to receive events from any spawned target.
	FOnTargetHitSignature OnTargetHit;
	FOnTargetMissedSignature OnTargetMissed;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Spawning")
	UBoxComponent* SpawnArea;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<ATarget> TargetClass;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	float MinSeparation = 100.0f;

private:
	// Targets spawned and not yet destroyed (used for separation + cleanup).
	UPROPERTY()
	TArray<ATarget*> ActiveTargets;

	// Bound to each spawned target; relays its event up via the delegates above.
	UFUNCTION()
	void HandleTargetHit(ATarget* HitTarget);

	UFUNCTION()
	void HandleTargetExpired(ATarget* ExpiredTarget);
};
