// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SarPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTANDRUN_API ASarPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/*
	 *	Replication notifies
	 */
	virtual void OnRep_Score() override;

	UFUNCTION()
	virtual void OnRep_Defeats();

	UFUNCTION()
	void OnRep_KilledBy();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastDeathMessage(const FString& KillerName);
	
	void AddToScore(float ScoreAmount);
	void AddToDefeats(int32 DefeatsAmount);
	void UpdateDeathMessage(FString KillerName);
	
private:
	void UpdateDeathMessageHUD();
	
	UPROPERTY()
	class ASarCharacter* Character;
	
	UPROPERTY()
	class ASarPlayerController* Controller;

	UPROPERTY(ReplicatedUsing=OnRep_Defeats)
	int32 Defeats;

	UPROPERTY(ReplicatedUsing=OnRep_KilledBy)
	FString KilledBy;
};
