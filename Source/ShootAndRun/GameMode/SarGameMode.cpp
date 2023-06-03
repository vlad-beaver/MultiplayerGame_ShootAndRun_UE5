// Fill out your copyright notice in the Description page of Project Settings.


#include "SarGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "ShootAndRun/Character/SarCharacter.h"
#include "ShootAndRun/PlayerController/SarPlayerController.h"
#include "ShootAndRun/PlayerState/SarPlayerState.h"
#include "ShootAndRun/GameState/SarGameState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

ASarGameMode::ASarGameMode()
{
	bDelayedStart = true;
}

void ASarGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ASarGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void ASarGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ASarPlayerController* SarPlayer = Cast<ASarPlayerController>(*It);
		if(SarPlayer)
		{
			SarPlayer->OnMatchStateSet(MatchState);
		}
	}
}

void ASarGameMode::PlayerEliminated(ASarCharacter* ElimmedCharacter, ASarPlayerController* VictimController,
                                    ASarPlayerController* AttackerController)
{
	ASarPlayerState* AttackerPlayerState = AttackerController ? Cast<ASarPlayerState>(AttackerController->PlayerState) : nullptr;
	ASarPlayerState* VictimPlayerState = VictimController ? Cast<ASarPlayerState>(VictimController->PlayerState) : nullptr;

	ASarGameState* SarGameState = GetGameState<ASarGameState>();
	
	//	Add kills to kill counter
	if(AttackerPlayerState && AttackerPlayerState != VictimPlayerState && SarGameState)
	{
		AttackerPlayerState->AddToScore(1.f);
		SarGameState->UpdateTopScore(AttackerPlayerState);
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	//	Display "Killed by player" message
	if (AttackerPlayerState && VictimPlayerState)
	{
		FString PlayerName = AttackerPlayerState->GetPlayerName();
		VictimPlayerState->UpdateDeathMessage(PlayerName);
	}
	
	
	if(ElimmedCharacter)
	{
		ElimmedCharacter->Elim(false);
	}
}

void ASarGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if(ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}

void ASarGameMode::PlayerLeftGame(ASarPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;
	ASarGameState* SarGameState = GetGameState<ASarGameState>();
	if (SarGameState && SarGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		SarGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	ASarCharacter* CharacterLeaving = Cast<ASarCharacter>(PlayerLeaving->GetPawn());
	if (CharacterLeaving)
	{
		CharacterLeaving->Elim(true);
	}
}
