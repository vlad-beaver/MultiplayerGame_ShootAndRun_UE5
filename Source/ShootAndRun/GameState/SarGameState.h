// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ShootAndRun/PlayerState/SarPlayerState.h"
#include "SarGameState.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTANDRUN_API ASarGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(class ASarPlayerState* ScoringPlayer);
	
	UPROPERTY(Replicated)
	TArray<ASarPlayerState*> TopScoringPlayers;

private:
	float TopScore = 0.f;
};
