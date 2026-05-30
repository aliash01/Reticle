// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Target.generated.h"

class USphereComponent;
class UCapsuleComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTargetHit, ATarget*, HitTarget, bool, bHeadshot);
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

	void HandleHit(const FHitResult& Hit);
	// NewLocation is relative to the attach parent (the SpawnManager) — local space.
	void Activate(const FVector& NewLocation, float LifeTime);
	void Deactivate();

	bool IsActivated() const { return bActivated; }

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere)
	UCapsuleComponent* BodyHitbox;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* HeadHitbox;

private:
	bool bActivated = false;

	FTimerHandle LifeCycleTimer;

	void OnLifetimeExpired();
};