#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CrosshairWidget.generated.h"

class UAimUserSettingsAsset;

UCLASS()
class RETICLE_API UCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Crosshair")
	void ApplyProfile(UAimUserSettingsAsset* InProfile);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	virtual int32 NativePaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Crosshair")
	TObjectPtr<UAimUserSettingsAsset> Profile;

private:
	void BindToProfile();
	void UnbindFromProfile();
	void HandleProfileChanged();

	FDelegateHandle ProfileChangedHandle;
	TWeakObjectPtr<UAimUserSettingsAsset> BoundProfile;
};
