// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Target.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetHit, ATarget*, HitTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetExpired, ATarget*, ExpiredTarget);

UCLASS(Abstract)
class RETICLE_API ATarget : public AActor
{
	GENERATED_BODY()

public:
	ATarget();

	UPROPERTY(BlueprintAssignable)
	FOnTargetHit OnTargetHit;

	UPROPERTY(BlueprintAssignable)
	FOnTargetExpired OnTargetExpired;

	// Virtual so a headshot variant (AHeadshotTarget) can override to add head/body detection.
	virtual void HandleHit(const FHitResult& Hit);
	// NewLocation is relative to the attach parent (the SpawnManager) — local space.
	void Activate(const FVector& NewLocation, float LifeTime);
	void Deactivate();

	bool IsActivated() const { return bActivated; }

	// Visual-only active/inactive state for subtests that show several targets at once
	// (e.g. Switching). The BP implements the look; no effect on collision or scoring.
	UFUNCTION(BlueprintImplementableEvent, Category = "Target")
	void SetHighlighted(bool bHighlighted);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* MeshComponent;

private:
	bool bActivated = false;

	FTimerHandle LifeCycleTimer;

	void OnLifetimeExpired();
};