#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SensConversionLibrary.generated.h"

class UDataTable;

UCLASS()
class RETICLE_API USensConversionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Degrees of camera rotation per raw mouse count, given the selected game and the user's in-game sens. */
	UFUNCTION(BlueprintPure, Category="Sensitivity")
	static float GetEffectiveDegPerCount(const UDataTable* ProfilesTable, FName GameRowName, float SourceSens);

	/** cm needed to perform a 360° turn, for the sanity-check readout. */
	UFUNCTION(BlueprintPure, Category="Sensitivity")
	static float CalcCmPer360(float DegPerCount, float DPI);
};
