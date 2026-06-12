#include "Common/FirstPersonPawnBase.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Sensitivity/ReticleSensSubsystem.h"

AFirstPersonPawnBase::AFirstPersonPawnBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	SetRootComponent(Camera);
	Camera->bUsePawnControlRotation = true;
}

void AFirstPersonPawnBase::BeginPlay()
{
	Super::BeginPlay();

	if (auto* PC = Cast<APlayerController>(GetController()))
	{
		if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}

		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UReticleSensSubsystem* Sens = GI->GetSubsystem<UReticleSensSubsystem>())
		{
			Sens->OnSensSettingsChanged.AddDynamic(this, &AFirstPersonPawnBase::RefreshSensCoefficient);
			RefreshSensCoefficient();
		}
	}
}

void AFirstPersonPawnBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (auto* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (LookAction)
		{
			EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFirstPersonPawnBase::OnLook);
		}
	}
}

void AFirstPersonPawnBase::OnLook(const FInputActionValue& Value)
{
	if (bLookLocked) return;
	if (CachedDegPerCount <= 0.f) return;

	const FVector2D Axis = Value.Get<FVector2D>();
	if (Axis.IsNearlyZero()) return;

	auto* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	FRotator R = PC->GetControlRotation();
	R.Yaw   += Axis.X * CachedDegPerCount;
	R.Pitch  = FMath::Clamp(R.Pitch + Axis.Y * CachedDegPerCount, -89.f, 89.f);
	PC->SetControlRotation(R);
}

void AFirstPersonPawnBase::RefreshSensCoefficient()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UReticleSensSubsystem* Sens = GI->GetSubsystem<UReticleSensSubsystem>())
		{
			const float Eff = Sens->GetEffectiveDegPerCount();
			CachedDegPerCount = Eff > 0.f ? Eff : 0.022f;
			UE_LOG(LogTemp, Warning, TEXT("[ReticleSens] Pawn refreshed: CachedDegPerCount=%.6f (raw=%.6f)"), CachedDegPerCount, Eff);
		}
	}
}
