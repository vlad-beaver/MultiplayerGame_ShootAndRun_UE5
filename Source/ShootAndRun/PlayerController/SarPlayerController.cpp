// Fill out your copyright notice in the Description page of Project Settings.


#include "SarPlayerController.h"

#include "Components/ProgressBar.h"
#include "ShootAndRun/HUD/SarHUD.h"
#include "ShootAndRun/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "ShootAndRun/Character/SarCharacter.h"

void ASarPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SarHUD = Cast<ASarHUD>(GetHUD());
}

void ASarPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ASarCharacter* SarCharacter = Cast<ASarCharacter>(InPawn);
	if (SarCharacter)
	{
		SetHUDHealth(SarCharacter->GetHealth(), SarCharacter->GetMaxHealth());
	}
}

void ASarPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	SarHUD = SarHUD == nullptr ? Cast<ASarHUD>(GetHUD()) : SarHUD;

	bool bHUDValid = SarHUD &&
		SarHUD->CharacterOverlay &&
			SarHUD->CharacterOverlay->HealthBar &&
				SarHUD->CharacterOverlay->HealthText;
	if(bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		SarHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		SarHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}
