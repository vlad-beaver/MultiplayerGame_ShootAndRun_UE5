// Fill out your copyright notice in the Description page of Project Settings.


#include "SarAnimInstance.h"

#include "SarCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

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
}
