#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AimUserSettingsAsset.generated.h"

class UDataTable;

UCLASS(BlueprintType)
class RETICLE_API UAimUserSettingsAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sensitivity")
	TObjectPtr<UDataTable> ProfilesTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sensitivity", meta=(GetOptions="GetGameRowOptions"))
	FName SelectedGameRowName = "CS2";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sensitivity", meta=(ClampMin="0.001"))
	float SourceGameSens = 1.0f;

#if WITH_EDITOR
	UFUNCTION()
	TArray<FName> GetGameRowOptions() const;
#endif
};
