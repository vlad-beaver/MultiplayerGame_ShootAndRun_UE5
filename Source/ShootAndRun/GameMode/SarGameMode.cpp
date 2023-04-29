// Fill out your copyright notice in the Description page of Project Settings.


#include "SarGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "ShootAndRun/Character/SarCharacter.h"
#include "ShootAndRun/PlayerController/SarPlayerController.h"
#include "ShootAndRun/PlayerState/SarPlayerState.h"

void ASarGameMode::PlayerEliminated(ASarCharacter* ElimmedCharacter, ASarPlayerController* VictimController,
	ASarPlayerController* AttackerController)
{
	ASarPlayerState* AttackerPlayerState = AttackerController ? Cast<ASarPlayerState>(AttackerController->PlayerState) : nullptr;
	ASarPlayerState* VictimPlayerState = VictimController ? Cast<ASarPlayerState>(VictimController->PlayerState) : nullptr;

	//	Add kills to kill counter
	if(AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
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
		ElimmedCharacter->Elim();
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
