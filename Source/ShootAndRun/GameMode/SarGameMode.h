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
	ASarGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(class ASarCharacter* ElimmedCharacter, class ASarPlayerController* VictimController, class ASarPlayerController* AttackerController);
	virtual void RequestRespawn(class ACharacter* ElimmedCharacter, AController* ElimmedController); 

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;
};
