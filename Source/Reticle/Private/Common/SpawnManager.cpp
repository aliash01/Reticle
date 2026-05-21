#include "Common/SpawnManager.h"

#include "Common/Target/Target.h"

ASpawnManager::ASpawnManager()
{
    PrimaryActorTick.bCanEverTick = false;

}

void ASpawnManager::OnLevelStart(int32 NumOfTargets, float SetTargetShowTime, float SetTargetHideTime)
{
    TargetShowTime = SetTargetShowTime;
    TargetHideTime = SetTargetHideTime;
    TargetsRemaining = NumOfTargets;
    TotalHits = 0;
    TotalMisses = 0;

    RandomStream.Initialize(bUseSeed ? Seed : FMath::Rand());

    // TODO: Countdown

    InitializePool();
    EnsureActiveTargets();
}

void ASpawnManager::InitializePool()
{
    if (!TargetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("TargetClass not set on SpawnManager"));
        return;
    }

    for (int32 i = 0; i < PoolSize; ++i)
    {
        ATarget* T = GetWorld()->SpawnActor<ATarget>(
            TargetClass, FVector::ZeroVector, FRotator::ZeroRotator);

        if (!T) continue;

        T->OnTargetHit.AddDynamic(this, &ASpawnManager::HandleTargetHit);
        T->OnTargetExpired.AddDynamic(this, &ASpawnManager::HandleTargetExpired);

        T->Deactivate();
        TargetPool.Add(T);
    }
}

void ASpawnManager::EnsureActiveTargets()
{
    int32 ActiveCount = CountActiveTargets();

    while (ActiveCount < MaxActiveTargets && TargetsRemaining > 0)
    {
        ATarget* T = FindInactiveTarget();
        if (!T) break;

        T->Activate(GenerateOnePosition(), TargetShowTime);
        --TargetsRemaining;
        ++ActiveCount;
    }

    if (ActiveCount == 0 && TargetsRemaining == 0)
    {
        OnLevelComplete();
    }
}

void ASpawnManager::ScheduleEnsureActive()
{
    FTimerHandle Handle;
    GetWorld()->GetTimerManager().SetTimer(
        Handle, this, &ASpawnManager::EnsureActiveTargets, TargetHideTime, false);
}

void ASpawnManager::OnLevelComplete()
{
}

int32 ASpawnManager::CountActiveTargets() const
{
    int32 Count = 0;
    for (ATarget* T : TargetPool)
    {
        if (T && T->IsActivated()) ++Count;
    }
    return Count;
}

ATarget* ASpawnManager::FindInactiveTarget() const
{
    for (ATarget* T : TargetPool)
    {
        if (T && !T->IsActivated()) return T;
    }
    return nullptr;
}

void ASpawnManager::HandleTargetHit(ATarget* HitTarget, bool bHeadshot)
{
    if (!HitTarget || !HitTarget->IsActivated()) return;

    HitTarget->Deactivate();
    ++TotalHits;

    ScheduleEnsureActive();
}

void ASpawnManager::HandleTargetExpired(ATarget* ExpiredTarget)
{
    if (!ExpiredTarget) return;

    ++TotalMisses;

    ScheduleEnsureActive();
}

TArray<FVector> ASpawnManager::GenerateSpawnPositions(int32 NumOfTargets)
{
    // Generate random positions between spawnOffset - -> + of x and y.
    // e.g spawn offset of (200, 200) means they can spawn between [-200,200] for x and [-200,200] for y
    TArray<FVector> Positions;
    Positions.Reserve(NumOfTargets);

    RandomStream.Initialize(bUseSeed ? Seed : FMath::Rand());

    for (int32 i = 0; i < NumOfTargets; ++i)
    {
       const float X = RandomStream.FRandRange(-SpawnOffset.X, SpawnOffset.X);
       const float Y = RandomStream.FRandRange(-SpawnOffset.Y, SpawnOffset.Y);
       Positions.Add(FVector(X, Y, 0.0f));
    }

    return Positions;
}

FVector ASpawnManager::GenerateOnePosition()
{
    constexpr int32 MaxAttempts = 20;

    FVector Candidate = FVector::ZeroVector;

    for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
    {
        const float X = RandomStream.FRandRange(-SpawnOffset.X, SpawnOffset.X);
        const float Y = RandomStream.FRandRange(-SpawnOffset.Y, SpawnOffset.Y);
        Candidate = FVector(X, Y, 0.0f);

        if (!IsTooCloseToActiveTarget(Candidate)) return Candidate;
    }

    return Candidate;
}

bool ASpawnManager::IsTooCloseToActiveTarget(const FVector& Pos) const
{
    for (ATarget* T : TargetPool)
    {
        if (T && T->IsActivated() &&
            FVector::Dist(T->GetActorLocation(), Pos) < MinSeparation)
        {
            return true;
        }
    }
    return false;
}