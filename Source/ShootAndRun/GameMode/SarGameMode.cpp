// Fill out your copyright notice in the Description page of Project Settings.


#include "SarGameMode.h"
#include "ShootAndRun/Character/SarCharacter.h"
#include "ShootAndRun/PlayerController/SarPlayerController.h"

void ASarGameMode::PlayerEliminated(ASarCharacter* ElimmedCharacter, ASarPlayerController* VictimController,
	ASarPlayerController* AttackerController)
{
	if(ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}
}
