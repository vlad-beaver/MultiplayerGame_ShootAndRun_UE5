#pragma once

#define TRACE_LENGTH 80000.f

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "AssaultRifle"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_GrenadeLauncher UMETA(DisplayName = "GrenadeLauncher"),
	
	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};