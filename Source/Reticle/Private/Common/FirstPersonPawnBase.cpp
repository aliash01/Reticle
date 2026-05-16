// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/FirstPersonPawnBase.h"

// Sets default values
AFirstPersonPawnBase::AFirstPersonPawnBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFirstPersonPawnBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFirstPersonPawnBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AFirstPersonPawnBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

