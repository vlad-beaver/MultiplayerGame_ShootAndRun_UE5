// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "SarGameMode.generated.h"

namespace MatchState
{
	extern SHOOTANDRUN_API const FName Cooldown;	//	Match duration has been reached. Display winner and begin cooldown timer.
}

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

	void PlayerLeftGame(class ASarPlayerState* PlayerLeaving);
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;
public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};
