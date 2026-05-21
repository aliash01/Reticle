#include "Sensitivity/AimUserSettingsAsset.h"

#include "Engine/DataTable.h"

#if WITH_EDITOR
TArray<FName> UAimUserSettingsAsset::GetGameRowOptions() const
{
	if (!ProfilesTable)
	{
		return {};
	}
	return ProfilesTable->GetRowNames();
}

void UAimUserSettingsAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	OnCrosshairChanged.Broadcast();
}
#endif
