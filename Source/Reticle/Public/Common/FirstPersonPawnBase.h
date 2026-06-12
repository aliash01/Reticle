#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "FirstPersonPawnBase.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS(Abstract)
class RETICLE_API AFirstPersonPawnBase : public APawn
{
	GENERATED_BODY()

public:
	AFirstPersonPawnBase();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void LockLook() { bLookLocked = true; }
	void UnlockLook() { bLookLocked = false; }
protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputAction> LookAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	void OnLook(const FInputActionValue& Value);

	UFUNCTION()
	void RefreshSensCoefficient();

	float CachedDegPerCount = 0.022f;

	bool bLookLocked = false;
};
