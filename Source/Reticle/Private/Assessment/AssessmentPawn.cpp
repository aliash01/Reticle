#include "Assessment/AssessmentPawn.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Common/Target/Target.h"
#include "TimerManager.h"

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
			EIC->BindAction(ShootAction, ETriggerEvent::Completed, this, &AAssessmentPawn::OnFireReleased);
		}
	}
}

void AAssessmentPawn::FireShot()
{
	UE_LOG(LogTemp, Warning, TEXT("Shoot!"));

	OnFire.Broadcast();
	
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

void AAssessmentPawn::OnShoot(const FInputActionValue& Value)
{
	bIsFiring = true;
	FireShot();   // immediate shot on press

	// Automatic mode keeps firing while held (tracking); semi-auto leaves this off.
	if (bAutomatic)
	{
		GetWorld()->GetTimerManager().SetTimer(
			FireTimer, this, &AAssessmentPawn::FireShot, FireInterval, true);
	}
}

void AAssessmentPawn::OnFireReleased()
{
	bIsFiring = false;
	GetWorld()->GetTimerManager().ClearTimer(FireTimer);
}

void AAssessmentPawn::SetAutomaticFire(bool bInAutomatic, float InFireInterval)
{
	bAutomatic = bInAutomatic;
	FireInterval = InFireInterval;

	if (!bAutomatic)
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimer);   // stop any in-progress auto-fire
	}
}