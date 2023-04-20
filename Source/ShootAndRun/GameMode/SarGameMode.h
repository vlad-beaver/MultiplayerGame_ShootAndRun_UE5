// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "SarGameMode.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTANDRUN_API ASarGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PlayerEliminated(class ASarCharacter* ElimmedCharacter, class ASarPlayerController* VictimController, class ASarPlayerController* AttackerController);
	virtual void RequestRespawn(class ACharacter* ElimmedCharacter, AController* ElimmedController); 

};
