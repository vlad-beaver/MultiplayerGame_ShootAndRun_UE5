// Fill out your copyright notice in the Description page of Project Settings.


#include "SarPlayerController.h"

#include "Components/ProgressBar.h"
#include "ShootAndRun/HUD/SarHUD.h"
#include "ShootAndRun/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "ShootAndRun/Character/SarCharacter.h"
#include "ShootAndRun/PlayerState/SarPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "ShootAndRun/GameMode/SarGameMode.h"
#include "ShootAndRun/HUD/Announcement.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnitConversion.h"

void ASarPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SarHUD = Cast<ASarHUD>(GetHUD());
	ServerCheckMatchState();
}

void ASarPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASarPlayerController, MatchState);
}

void ASarPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
}

void ASarPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ASarPlayerController::ServerCheckMatchState_Implementation()
{
	ASarGameMode* GameMode = Cast<ASarGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, LevelStartingTime);

		if(SarHUD && MatchState == MatchState::WaitingToStart)
		{
			SarHUD->AddAnnouncement();
		}
	}
}

void ASarPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
	if(SarHUD && MatchState == MatchState::WaitingToStart)
	{
		SarHUD->AddAnnouncement();
	}
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
	else
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
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
	else
	{
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
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
	else
	{
		bInitializeCharacterOverlay = true;
		HUDDefeats = Defeats;
	}
}

void ASarPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	SarHUD = SarHUD == nullptr ? Cast<ASarHUD>(GetHUD()) : SarHUD;
	bool bHUDValid = SarHUD &&
		SarHUD->CharacterOverlay &&
			SarHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		SarHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ASarPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	SarHUD = SarHUD == nullptr ? Cast<ASarHUD>(GetHUD()) : SarHUD;
	bool bHUDValid = SarHUD &&
		SarHUD->CharacterOverlay &&
			SarHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		SarHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ASarPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	SarHUD = SarHUD == nullptr ? Cast<ASarHUD>(GetHUD()) : SarHUD;
	bool bHUDValid = SarHUD &&
		SarHUD->CharacterOverlay &&
			SarHUD->CharacterOverlay->MatchCountdownText;
	if (bHUDValid)
	{
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;
		
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		SarHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void ASarPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	SarHUD = SarHUD == nullptr ? Cast<ASarHUD>(GetHUD()) : SarHUD;
	bool bHUDValid = SarHUD &&
			SarHUD->Announcement &&
			SarHUD->Announcement->WarmupTime;
	if (bHUDValid)
	{
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;
		
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		SarHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void ASarPlayerController::SetHUDTime()
{
	if(HasAuthority())
	{
		ASarGameMode* SarGameMode = Cast<ASarGameMode>(UGameplayStatics::GetGameMode(this));
		if(SarGameMode)
		{
			LevelStartingTime = SarGameMode->LevelStartingTime;
		}
	}
	
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}
	
	CountdownInt = SecondsLeft;
}

void ASarPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (SarHUD && SarHUD->CharacterOverlay)
		{
			CharacterOverlay = SarHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);
			}
		}
	}
}

void ASarPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ASarPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
	float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float ASarPlayerController::GetServerTime()
{
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ASarPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ASarPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;
	
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
}

void ASarPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
}

void ASarPlayerController::HandleMatchHasStarted()
{
	SarHUD = SarHUD == nullptr ? Cast<ASarHUD>(GetHUD()) : SarHUD;
	if (SarHUD)
	{
		SarHUD->AddCharacterOverlay();
		if (SarHUD->Announcement)
		{
			SarHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
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
