#pragma once

#include "CoreMinimal.h"
#include "Common/FirstPersonPawnBase.h"
#include "AssessmentPawn.generated.h"

class UInputAction;
struct FInputActionValue;

UCLASS()
class RETICLE_API AAssessmentPawn : public AFirstPersonPawnBase
{
	GENERATED_BODY()

public:
	AAssessmentPawn();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputAction> ShootAction;

	void OnShoot(const FInputActionValue& Value);
};