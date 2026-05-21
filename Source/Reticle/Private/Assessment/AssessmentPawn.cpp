#include "Assessment/AssessmentPawn.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Common/Target/Target.h"

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
	UE_LOG(LogTemp, Warning, TEXT("Shoot!"));
	FVector CameraLocation;
	FRotator CameraRotation;
	GetController()->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector Start = CameraLocation;
	FVector Direction = CameraRotation.Vector();
	FVector End = Start + Direction * MaxRange;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());  // don't shoot yourself

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_Visibility, Params);
	
	if (bHit)
	{
		if (ATarget* Target = Cast<ATarget>(Hit.GetActor()))
		{ 
			Target->HandleHit(Hit);
		}
	}
}