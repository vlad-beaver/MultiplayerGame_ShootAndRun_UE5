// Fill out your copyright notice in the Description page of Project Settings.


#include "SarPlayerController.h"

#include "Components/ProgressBar.h"
#include "ShootAndRun/HUD/SarHUD.h"
#include "ShootAndRun/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "ShootAndRun/Character/SarCharacter.h"
#include "ShootAndRun/PlayerState/SarPlayerState.h"

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
	HideDeathMessage();
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

void ASarPlayerController::SetHUDScore(float Score)
{
	SarHUD = SarHUD == nullptr ? Cast<ASarHUD>(GetHUD()) : SarHUD;
	bool bHUDValid = SarHUD &&
		SarHUD->CharacterOverlay &&
			SarHUD->CharacterOverlay->ScoreAmount;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		SarHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
}

void ASarPlayerController::SetHUDDefeats(int32 Defeats)
{
	SarHUD = SarHUD == nullptr ? Cast<ASarHUD>(GetHUD()) : SarHUD;
	bool bHUDValid = SarHUD &&
		SarHUD->CharacterOverlay &&
			SarHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		SarHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
}

void ASarPlayerController::UpdateDeathMessage(const FString KilledBy)
{
	SarHUD = SarHUD == nullptr ? Cast<ASarHUD>(GetHUD()) : SarHUD;
	if (SarHUD &&
		SarHUD->CharacterOverlay &&
		SarHUD->CharacterOverlay->DeathMessage &&
		SarHUD->CharacterOverlay->KilledBy)
	{
		SarHUD->CharacterOverlay->KilledBy->SetText(FText::FromString(KilledBy));
		SarHUD->CharacterOverlay->KilledBy->SetVisibility(ESlateVisibility::Visible);
		SarHUD->CharacterOverlay->DeathMessage->SetVisibility(ESlateVisibility::Visible);
	}
}

void ASarPlayerController::HideDeathMessage()
{
	SarHUD = SarHUD == nullptr ? Cast<ASarHUD>(GetHUD()) : SarHUD;
	if (SarHUD &&
		SarHUD->CharacterOverlay &&
		SarHUD->CharacterOverlay->DeathMessage &&
		SarHUD->CharacterOverlay->KilledBy)
	{
		SarHUD->CharacterOverlay->KilledBy->SetVisibility(ESlateVisibility::Collapsed);
		SarHUD->CharacterOverlay->DeathMessage->SetVisibility(ESlateVisibility::Collapsed);
	}
}
