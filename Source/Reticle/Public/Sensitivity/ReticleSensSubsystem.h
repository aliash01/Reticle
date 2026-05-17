#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ReticleSensSubsystem.generated.h"

class UAimUserSettingsAsset;
class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSensSettingsChanged);

UCLASS(Config=Game)
class RETICLE_API UReticleSensSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintPure, Category="Sensitivity")
	UDataTable* GetProfilesTable() const;

	UFUNCTION(BlueprintPure, Category="Sensitivity")
	FName GetSelectedGameRowName() const { return CachedGameRowName; }

	UFUNCTION(BlueprintPure, Category="Sensitivity")
	float GetSourceGameSens() const { return CachedSourceSens; }

	UFUNCTION(BlueprintPure, Category="Sensitivity")
	float GetEffectiveDegPerCount() const;

	/** Runtime-only override (no persistence). When a widget exists later, wire it through this. */
	UFUNCTION(BlueprintCallable, Category="Sensitivity")
	void SetSelection(FName GameRowName, float Sens);

	UPROPERTY(BlueprintAssignable, Category="Sensitivity")
	FOnSensSettingsChanged OnSensSettingsChanged;

protected:
	/** Asset that holds the user's selection. Path set in DefaultGame.ini. */
	UPROPERTY(Config)
	FSoftObjectPath UserSettingsAsset;

	UPROPERTY(Transient)
	TObjectPtr<UAimUserSettingsAsset> CachedAsset;

	UPROPERTY(Transient)
	FName CachedGameRowName = "CS2";

	UPROPERTY(Transient)
	float CachedSourceSens = 1.0f;
};
