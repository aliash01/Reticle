// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/Target/Target.h"

#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
ATarget::ATarget()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = MeshComponent;
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    BodyHitbox = CreateDefaultSubobject<UCapsuleComponent>(TEXT("BodyHitbox"));
    BodyHitbox->SetupAttachment(MeshComponent);
    BodyHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    BodyHitbox->SetCollisionResponseToAllChannels(ECR_Block);

    HeadHitbox = CreateDefaultSubobject<USphereComponent>(TEXT("HeadHitbox"));
    HeadHitbox->SetupAttachment(MeshComponent);
    HeadHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    HeadHitbox->SetCollisionResponseToAllChannels(ECR_Block);
}

void ATarget::HandleHit(const FHitResult& Hit)
{
    if (Hit.GetComponent() == HeadHitbox)
    {
       // headshot damage / score
       OnTargetHit.Broadcast(this, true);
    }
    else if (Hit.GetComponent() == BodyHitbox)
    {
       // body damage / score
       OnTargetHit.Broadcast(this, false);
    }
}

void ATarget::Activate(const FVector& NewLocation, float LifeTime)
{
    FTimerManager& TM = GetWorld()->GetTimerManager();
    TM.ClearTimer(LifeCycleTimer);

    bActivated = true;
    SetActorLocation(NewLocation);
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