// Fill out your copyright notice in the Description page of Project Settings.

#include "Common/SpawnManager.h"

#include "Assessment/AssessmentGameMode.h"
#include "Common/Target/Target.h"
#include "Components/BoxComponent.h"

ASpawnManager::ASpawnManager()
{
    PrimaryActorTick.bCanEverTick = false;

    SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
    RootComponent = SpawnArea;
    SpawnArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SpawnArea->SetHiddenInGame(true);
    SpawnArea->ShapeColor = FColor::Green;
    SpawnArea->SetBoxExtent(FVector(200.f, 200.f, 50.f));
}

void ASpawnManager::BeginPlay()
{
    Super::BeginPlay();

    if (AAssessmentGameMode* GM = GetWorld()->GetAuthGameMode<AAssessmentGameMode>())
    {
        GM->RegisterActiveSpawnManager(this);
    }
}

ATarget* ASpawnManager::SpawnTarget(const FVector& LocalLocation, float LifeTime)
{
    if (!TargetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("TargetClass not set on SpawnManager"));
        return nullptr;
    }

    ATarget* T = GetWorld()->SpawnActor<ATarget>(
        TargetClass, GetActorLocation(), FRotator::ZeroRotator);

    if (!T) return nullptr;

    // Parent to this manager so the target's frame IS the manager; Activate then
    // places it via relative location ((0,0,0) == manager centre).
    T->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);

    T->OnTargetHit.AddDynamic(this, &ASpawnManager::HandleTargetHit);
    T->OnTargetExpired.AddDynamic(this, &ASpawnManager::HandleTargetExpired);

    // LifeTime <= 0 => Activate's timer never fires, so the target persists
    // until the owner calls DestroyTarget.
    T->Activate(LocalLocation, LifeTime);
    ActiveTargets.Add(T);
    return T;
}

void ASpawnManager::DestroyTarget(ATarget* Target)
{
    if (!Target) return;

    Target->OnTargetHit.RemoveDynamic(this, &ASpawnManager::HandleTargetHit);
    Target->OnTargetExpired.RemoveDynamic(this, &ASpawnManager::HandleTargetExpired);

    ActiveTargets.Remove(Target);
    Target->Destroy();
}

void ASpawnManager::HandleTargetHit(ATarget* HitTarget, bool bHeadshot)
{
    // Relay up; the owner decides what a hit means and when to despawn.
    OnTargetHit.Broadcast(HitTarget, bHeadshot);
}

void ASpawnManager::HandleTargetExpired(ATarget* ExpiredTarget)
{
    // Target has already self-deactivated; relay the miss up to the owner.
    OnTargetMissed.Broadcast(ExpiredTarget);
}

FVector ASpawnManager::GeneratePosition(FRandomStream& Stream) const
{
    constexpr int32 MaxAttempts = 20;

    const FVector Extent = SpawnArea->GetUnscaledBoxExtent();
    const FTransform Xform = GetActorTransform();

    FVector Local = FVector::ZeroVector;

    for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
    {
        Local.X = Stream.FRandRange(-Extent.X, Extent.X);
        Local.Y = Stream.FRandRange(-Extent.Y, Extent.Y);
        Local.Z = Stream.FRandRange(-Extent.Z, Extent.Z);

        // Separation is a world-space distance, so test the world position,
        // but return the local point so it pairs with SpawnTarget.
        if (!IsTooCloseToActiveTarget(Xform.TransformPosition(Local))) return Local;
    }

    return Local;
}

bool ASpawnManager::IsTooCloseToActiveTarget(const FVector& Location) const
{
    for (ATarget* T : ActiveTargets)
    {
        if (T && T->IsActivated() &&
            FVector::Dist(T->GetActorLocation(), Location) < MinSeparation)
        {
            return true;
        }
    }
    return false;
}
