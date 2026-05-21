#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AimUserSettingsAsset.generated.h"

class UDataTable;

UENUM(BlueprintType)
enum class ECrosshairShape : uint8
{
	Dot     UMETA(DisplayName="Dot"),
	Cross   UMETA(DisplayName="Cross (with gap)"),
	Plus    UMETA(DisplayName="Plus (no gap)"),
	Circle  UMETA(DisplayName="Circle"),
	TShape  UMETA(DisplayName="T-Shape"),
};

USTRUCT(BlueprintType)
struct FCrosshairSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shape")
	ECrosshairShape Shape = ECrosshairShape::Cross;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shape")
	FLinearColor Color = FLinearColor(0.f, 1.f, 0.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shape", meta=(ClampMin="0", ClampMax="1"))
	float MasterOpacity = 1.f;

	// Inner lines (Cross / Plus / T-Shape)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inner Lines", meta=(ClampMin="0", ClampMax="100"))
	float InnerLineLength = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inner Lines", meta=(ClampMin="0", ClampMax="20"))
	float InnerLineThickness = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inner Lines", meta=(ClampMin="0", ClampMax="1"))
	float InnerLineOpacity = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inner Lines", meta=(ClampMin="0", ClampMax="100"))
	float CenterGap = 4.f;

	// Outer lines (optional second tier, drawn beyond inner lines)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Outer Lines")
	bool bShowOuterLines = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Outer Lines", meta=(EditCondition="bShowOuterLines", ClampMin="0", ClampMax="100"))
	float OuterLineLength = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Outer Lines", meta=(EditCondition="bShowOuterLines", ClampMin="0", ClampMax="20"))
	float OuterLineThickness = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Outer Lines", meta=(EditCondition="bShowOuterLines", ClampMin="0", ClampMax="1"))
	float OuterLineOpacity = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Outer Lines", meta=(EditCondition="bShowOuterLines", ClampMin="0", ClampMax="100"))
	float OuterLineOffset = 4.f;

	// Center dot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Center Dot")
	bool bShowCenterDot = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Center Dot", meta=(EditCondition="bShowCenterDot", ClampMin="0", ClampMax="20"))
	float CenterDotSize = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Center Dot", meta=(EditCondition="bShowCenterDot", ClampMin="0", ClampMax="1"))
	float CenterDotOpacity = 1.f;

	// Circle shape
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Circle", meta=(ClampMin="1", ClampMax="200"))
	float CircleRadius = 12.f;

	// Outline (background stroke for visibility on busy scenes)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Outline")
	bool bOutline = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Outline", meta=(EditCondition="bOutline"))
	FLinearColor OutlineColor = FLinearColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Outline", meta=(EditCondition="bOutline", ClampMin="0", ClampMax="5"))
	float OutlineThickness = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Outline", meta=(EditCondition="bOutline", ClampMin="0", ClampMax="1"))
	float OutlineOpacity = 1.f;
};

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crosshair", meta=(ShowOnlyInnerProperties))
	FCrosshairSettings Crosshair;

	DECLARE_MULTICAST_DELEGATE(FOnCrosshairChanged);
	FOnCrosshairChanged OnCrosshairChanged;

#if WITH_EDITOR
	UFUNCTION()
	TArray<FName> GetGameRowOptions() const;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
