// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/Target/Target.h"

// Sets default values
ATarget::ATarget()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = MeshComponent;

    // The mesh's own simple collision IS the hitbox, so the hittable shape always
    // matches the assigned mesh (sphere, cylinder, ...) with no per-shape subclass.
    // QueryOnly: traces/overlaps hit it, but it never physically blocks movement.
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
}

void ATarget::HandleHit(const FHitResult& Hit)
{
    // Base targets have no head/body split — any hit is just a hit. A headshot
    // variant (AHeadshotTarget) overrides this to add a head hitbox and its own
    // headshot signal, leaving the base delegate headshot-free.
    OnTargetHit.Broadcast(this);
}

void ATarget::Activate(const FVector& NewLocation, float LifeTime)
{
    FTimerManager& TM = GetWorld()->GetTimerManager();
    TM.ClearTimer(LifeCycleTimer);

    bActivated = true;
    // Parent-relative: when attached to the SpawnManager, NewLocation is an
    // offset from the manager ((0,0,0) == its centre). Falls back to world if
    // the target has no attach parent.
    SetActorRelativeLocation(NewLocation);
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);

    TM.SetTimer(LifeCycleTimer, this, &ATarget::OnLifetimeExpired, LifeTime, false);
}

void ATarget::Deactivate()
{
    FTimerManager& TM = GetWorld()->GetTimerManager();
    TM.ClearTimer(LifeCycleTimer);

    bActivated = false;
    SetActorHiddenInGame(true);          // invisible
    SetActorEnableCollision(false);      // not hittable
    SetActorTickEnabled(false);          // skip Tick() for perf
}

void ATarget::OnLifetimeExpired()
{
    OnTargetExpired.Broadcast(this);
    Deactivate();
}