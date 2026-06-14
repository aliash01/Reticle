#pragma once

#include "CoreMinimal.h"
#include "Common/FirstPersonPawnBase.h"
#include "AssessmentPawn.generated.h"

class UInputAction;
struct FInputActionValue;

DECLARE_MULTICAST_DELEGATE(FOnFireSignature);

UCLASS()
class RETICLE_API AAssessmentPawn : public AFirstPersonPawnBase
{
	GENERATED_BODY()

public:
	AAssessmentPawn();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	FOnFireSignature OnFire;

	bool IsFiring() const { return bIsFiring; }

	// Configure fire mode: bInAutomatic = hold-to-fire at InFireInterval (e.g. tracking);
	// semi-auto (one shot per press) is the default.
	void SetAutomaticFire(bool bInAutomatic, float InFireInterval);
protected:
	UPROPERTY(EditAnywhere, Category = "Weapon")
	float MaxRange = 10000.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputAction> ShootAction;

	void OnShoot(const FInputActionValue& Value);
	void OnFireReleased();
	void FireShot();

private:
	bool bIsFiring = false;
	bool bAutomatic = false;
	float FireInterval = 0.1f;
	FTimerHandle FireTimer;
};