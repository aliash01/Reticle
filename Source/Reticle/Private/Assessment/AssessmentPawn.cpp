#include "Assessment/AssessmentPawn.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

AAssessmentPawn::AAssessmentPawn()
{
	if (Camera)
	{
		Camera->SetFieldOfView(90.0f);
	}
}

void AAssessmentPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (auto* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ShootAction)
		{
			EIC->BindAction(ShootAction, ETriggerEvent::Started, this, &AAssessmentPawn::OnShoot);
		}
	}
}

void AAssessmentPawn::OnShoot(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Log, TEXT("Shoot!"));
}