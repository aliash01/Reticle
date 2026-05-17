#include "Sensitivity/ReticleSensSubsystem.h"

#include "Engine/DataTable.h"
#include "Sensitivity/AimUserSettingsAsset.h"
#include "Sensitivity/SensConversionLibrary.h"

void UReticleSensSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadConfig();

	if (UserSettingsAsset.IsValid())
	{
		CachedAsset = Cast<UAimUserSettingsAsset>(UserSettingsAsset.TryLoad());
	}

	if (CachedAsset)
	{
		CachedGameRowName = CachedAsset->SelectedGameRowName;
		CachedSourceSens = CachedAsset->SourceGameSens;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ReticleSens] UserSettingsAsset not set or failed to load (path='%s'). Using defaults."), *UserSettingsAsset.ToString());
	}

	UDataTable* DT = GetProfilesTable();
	const float Eff = GetEffectiveDegPerCount();
	UE_LOG(LogTemp, Warning, TEXT("[ReticleSens] Init complete: asset='%s', table='%s', gameRow='%s', sens=%.4f, effectiveDegPerCount=%.6f"),
		CachedAsset ? *CachedAsset->GetName() : TEXT("<null>"),
		DT ? *DT->GetName() : TEXT("<null>"),
		*CachedGameRowName.ToString(),
		CachedSourceSens,
		Eff);

	OnSensSettingsChanged.Broadcast();
}

UDataTable* UReticleSensSubsystem::GetProfilesTable() const
{
	return CachedAsset ? CachedAsset->ProfilesTable.Get() : nullptr;
}

float UReticleSensSubsystem::GetEffectiveDegPerCount() const
{
	return USensConversionLibrary::GetEffectiveDegPerCount(GetProfilesTable(), CachedGameRowName, CachedSourceSens);
}

void UReticleSensSubsystem::SetSelection(FName GameRowName, float Sens)
{
	const bool bChanged = (GameRowName != CachedGameRowName) || !FMath::IsNearlyEqual(Sens, CachedSourceSens);
	CachedGameRowName = GameRowName;
	CachedSourceSens = FMath::Max(Sens, 0.f);
	if (bChanged)
	{
		OnSensSettingsChanged.Broadcast();
	}
}
