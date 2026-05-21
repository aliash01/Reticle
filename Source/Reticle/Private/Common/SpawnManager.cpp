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

FVector ASpawnManager::GenerateOnePosition()
{
    constexpr int32 MaxAttempts = 20;

    const FVector Extent = SpawnArea->GetUnscaledBoxExtent();
    const FTransform& BoxXform = SpawnArea->GetComponentTransform();

    FVector Candidate = BoxXform.GetLocation();

    for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
    {
        const float X = RandomStream.FRandRange(-Extent.X, Extent.X);
        const float Y = RandomStream.FRandRange(-Extent.Y, Extent.Y);
        const float Z = RandomStream.FRandRange(-Extent.Z, Extent.Z);
        Candidate = BoxXform.TransformPosition(FVector(X, Y, Z));

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