#include "Sensitivity/SensConversionLibrary.h"

#include "Engine/DataTable.h"
#include "Sensitivity/GameSensProfile.h"

float USensConversionLibrary::GetEffectiveDegPerCount(const UDataTable* ProfilesTable, FName GameRowName, float SourceSens)
{
	if (!ProfilesTable || GameRowName.IsNone() || SourceSens <= 0.f)
	{
		return 0.f;
	}

	const FGameSensProfile* Row = ProfilesTable->FindRow<FGameSensProfile>(GameRowName, TEXT("GetEffectiveDegPerCount"));
	if (!Row)
	{
		UE_LOG(LogTemp, Warning, TEXT("SensConversion: row '%s' not found in profile table"), *GameRowName.ToString());
		return 0.f;
	}

	return Row->DegPerCountAtSens1 * SourceSens;
}

float USensConversionLibrary::CalcCmPer360(float DegPerCount, float DPI)
{
	if (DegPerCount <= 0.f || DPI <= 0.f)
	{
		return 0.f;
	}
	const float CountsPer360 = 360.f / DegPerCount;
	const float InchesPer360 = CountsPer360 / DPI;
	return InchesPer360 * 2.54f;
}
