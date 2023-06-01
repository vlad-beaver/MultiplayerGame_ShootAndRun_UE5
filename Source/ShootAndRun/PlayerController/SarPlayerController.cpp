// Fill out your copyright notice in the Description page of Project Settings.


#include "SarPlayerController.h"

#include "Components/Image.h"
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
#include "ShootAndRun/SarComponents/CombatComponent.h"
#include "ShootAndRun/GameState/SarGameState.h"
#include "Components/Image.h"

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
	CheckPing(DeltaTime);
}

void ASarPlayerController::CheckPing(float DeltaTime)
{
	if (HasAuthority()) return;
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
		if (PlayerState)
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayerState->GetPing() * 4 : %d"), PlayerState->GetPing() * 4);
			if (PlayerState->GetPing() * 4 > HighPingThreshold)	//	Ping is compressed; it's actually ping / 4
				{
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
				ServerReportPingStatus(true);
				}
			else
			{
				ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0.f;
	}
	bool bHighPingAnimationPlaying =
		SarHUD && SarHUD->CharacterOverlay &&
			SarHUD->CharacterOverlay->HighPingAnimation &&
				SarHUD->CharacterOverlay->IsAnimationPlaying(SarHUD->CharacterOverlay->HighPingAnimation);
	if (bHighPingAnimationPlaying)
	{
		PingAnimationRunningTime += DeltaTime;
		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

// Is the ping too high?
void ASarPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
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

void ASarPlayerController::HighPingWarning()
{
	SarHUD = SarHUD == nullptr ? Cast<ASarHUD>(GetHUD()) : SarHUD;
	bool bHUDValid = SarHUD &&
		SarHUD->CharacterOverlay &&
			SarHUD->CharacterOverlay->HighPingImage &&
				SarHUD->CharacterOverlay->HighPingAnimation;
	if(bHUDValid)
	{
		SarHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		SarHUD->CharacterOverlay->PlayAnimation(
			SarHUD->CharacterOverlay->HighPingAnimation,
			0.f,
			5);
	}
}

void ASarPlayerController::StopHighPingWarning()
{
	SarHUD = SarHUD == nullptr ? Cast<ASarHUD>(GetHUD()) : SarHUD;
	bool bHUDValid = SarHUD &&
		SarHUD->CharacterOverlay &&
			SarHUD->CharacterOverlay->HighPingImage &&
				SarHUD->CharacterOverlay->HighPingAnimation;
	if(bHUDValid)
	{
		SarHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		if (SarHUD->CharacterOverlay->IsAnimationPlaying(SarHUD->CharacterOverlay->HighPingAnimation))
		{
			SarHUD->CharacterOverlay->StopAnimation(SarHUD->CharacterOverlay->HighPingAnimation);
		}
	}
}

void ASarPlayerController::ServerCheckMatchState_Implementation()
{
	ASarGameMode* GameMode = Cast<ASarGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);

		if(SarHUD && MatchState == MatchState::WaitingToStart)
		{
			SarHUD->AddAnnouncement();
		}
	}
}

void ASarPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
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
		bInitializeHealth = true;
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
		bInitializeScore = true;
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
		bInitializeDefeats = true;
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
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
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
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
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
		if (CountdownTime < 0.f)
		{
			SarHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}
		
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
		if (CountdownTime < 0.f)
		{
			SarHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
		
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
		SarGameMode = SarGameMode == nullptr ? Cast<ASarGameMode>(UGameplayStatics::GetGameMode(this)) : SarGameMode;
		if(SarGameMode)
		{
			LevelStartingTime = SarGameMode->LevelStartingTime;
		}
	}
	
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (HasAuthority())
	{
		SarGameMode = SarGameMode == nullptr ? Cast<ASarGameMode>(UGameplayStatics::GetGameMode(this)) : SarGameMode;
		if (SarGameMode)
		{
			SecondsLeft = FMath::CeilToInt(SarGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}
	
	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
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
				if (bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
				if (bInitializeScore) SetHUDScore(HUDScore);
				if (bInitializeDefeats) SetHUDDefeats(HUDDefeats);
				if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
				if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);
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
	SingleTripTime = 0.5f * RoundTripTime;
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
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
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ASarPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ASarPlayerController::HandleMatchHasStarted()
{
	SarHUD = SarHUD == nullptr ? Cast<ASarHUD>(GetHUD()) : SarHUD;
	if (SarHUD)
	{
		if (SarHUD->CharacterOverlay == nullptr) SarHUD->AddCharacterOverlay();
		if (SarHUD->Announcement)
		{
			SarHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ASarPlayerController::HandleCooldown()
{
	SarHUD = SarHUD == nullptr ? Cast<ASarHUD>(GetHUD()) : SarHUD;
	if (SarHUD)
	{
		SarHUD->CharacterOverlay->RemoveFromParent();
		bool bHUDValid = SarHUD->Announcement &&
				SarHUD->Announcement->AnnouncementText &&
				SarHUD->Announcement->InfoText;
		
		if (bHUDValid)
		{
			SarHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("New Match Starts In:");
			SarHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			ASarGameState* SarGameState = Cast<ASarGameState>(UGameplayStatics::GetGameState(this));
			ASarPlayerState* SarPlayerState = GetPlayerState<ASarPlayerState>(); 
			if (SarGameState && SarPlayerState)
			{
				TArray<ASarPlayerState*> TopPlayers = SarGameState->TopScoringPlayers;
				FString InfoTextString;
				if (TopPlayers.Num() == 0)
				{
					InfoTextString = FString("There is no winner.");
				}
				else if (TopPlayers.Num() == 1 && TopPlayers[0] == SarPlayerState)
				{
					InfoTextString = FString("You are the winner!");
				}
				else if (TopPlayers.Num() == 1)
				{
					InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *TopPlayers[0]->GetPlayerName());
				}
				else if (TopPlayers.Num() > 1)
				{
					InfoTextString = FString("Players tied for the win:\n");
					for (auto TiedPlayer : TopPlayers)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
					}
				}
				
				SarHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}
	ASarCharacter* SarCharacter = Cast<ASarCharacter>(GetPawn());
	if (SarCharacter && SarCharacter->GetCombat())
	{
		SarCharacter->bDisableGameplay = true;
		SarCharacter->GetCombat()->FireButtonPressed(false);
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
