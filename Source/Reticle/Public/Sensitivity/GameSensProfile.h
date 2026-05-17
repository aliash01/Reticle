#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameSensProfile.generated.h"

USTRUCT(BlueprintType)
struct RETICLE_API FGameSensProfile : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sensitivity")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sensitivity", meta=(ClampMin="0.0"))
	float DegPerCountAtSens1 = 0.022f;
};
