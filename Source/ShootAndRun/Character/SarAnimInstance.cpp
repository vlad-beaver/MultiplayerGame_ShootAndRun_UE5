// Fill out your copyright notice in the Description page of Project Settings.


#include "SarAnimInstance.h"

#include "SarCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void USarAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	SarCharacter = Cast<ASarCharacter>(TryGetPawnOwner());
}

void USarAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (SarCharacter == nullptr)
	{
		SarCharacter = Cast<ASarCharacter>(TryGetPawnOwner());
	}
	if (SarCharacter == nullptr) return;

	FVector Velocity = SarCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = SarCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = SarCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = SarCharacter->IsWeaponEquipped();
	bIsCrouched = SarCharacter->bIsCrouched;
	bAiming = SarCharacter->IsAiming();

	// Offset Yaw for Strafing
	FRotator AimRotation = SarCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(SarCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = SarCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = SarCharacter->GetAO_Yaw();
	AO_Pitch = SarCharacter->GetAO_Pitch();
}
