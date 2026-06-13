// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Subtests/SubtestBase.h"
#include "AssessmentGameMode.generated.h"

class UReactionTimeConfig;
class UFlickConfig;
class USubtestBase;
class ASpawnManager;
/**
 * 
 */
UCLASS()
class RETICLE_API AAssessmentGameMode : public AGameModeBase
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<ASpawnManager> ActiveSpawnManager;

public:
	void RegisterActiveSpawnManager(ASpawnManager* SpawnManager);
	virtual void BeginPlay() override;
	void StartReactionTimeSubtest();
	void StartFlickSubtest();
	void HandleSubtestEnded(const FSubtestResult& Result);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Subtest Config")
	UReactionTimeConfig* ReactionTimeConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Subtest Config")
	UFlickConfig* FlickConfig;


private:
	UPROPERTY() USubtestBase* ActiveSubtest = nullptr;
};
