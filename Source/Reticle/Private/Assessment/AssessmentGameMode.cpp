// Fill out your copyright notice in the Description page of Project Settings.


#include "Assessment/AssessmentGameMode.h"

#include "Common/SpawnManager.h"

void AAssessmentGameMode::RegisterActiveSpawnManager(ASpawnManager* SpawnManager)
{
	ActiveSpawnManager = SpawnManager;
}

void AAssessmentGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (ActiveSpawnManager)
	{
		ActiveSpawnManager->OnLevelStart(10, 2.f, 0.5f);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No SpawnManager registered"));
	}
}