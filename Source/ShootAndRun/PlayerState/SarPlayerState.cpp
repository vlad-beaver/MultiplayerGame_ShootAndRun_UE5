// Fill out your copyright notice in the Description page of Project Settings.


#include "SarPlayerState.h"
#include "ShootAndRun/Character/SarCharacter.h"
#include "ShootAndRun/PlayerController/SarPlayerController.h"
#include "Net/UnrealNetwork.h"

void ASarPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASarPlayerState, Defeats);
	DOREPLIFETIME(ASarPlayerState, KilledBy);
}


void ASarPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	Character = Character == nullptr ? Cast<ASarCharacter>( GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ASarPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void ASarPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<ASarCharacter>( GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ASarPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void ASarPlayerState::UpdateDeathMessage(FString KillerName)
{
	MulticastDeathMessage(KillerName);
}

void ASarPlayerState::UpdateDeathMessageHUD()
{
	Character = Character == nullptr ? Cast<ASarCharacter>( GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ASarPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->UpdateDeathMessage(KilledBy);
		}
	}
}

void ASarPlayerState::OnRep_KilledBy()
{
	UpdateDeathMessageHUD();
}

void ASarPlayerState::MulticastDeathMessage_Implementation(const FString& KillerName)
{
	Character = Character == nullptr ? Cast<ASarCharacter>( GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ASarPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->UpdateDeathMessage(KillerName);
		}
	}
}

void ASarPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	Character = Character == nullptr ? Cast<ASarCharacter>( GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ASarPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void ASarPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<ASarCharacter>( GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ASarPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}
