// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShootAndRun/Character/SarCharacter.h"
#include "LagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;

	UPROPERTY()
	ASarCharacter* Character;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTANDRUN_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend class ASarCharacter;
	
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);

	/** 
	* Hitscan
	*/
	FServerSideRewindResult ServerSideRewind(
		class ASarCharacter* HitCharacter, 
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize& HitLocation, 
		float HitTime);

	/** 
	* Projectile
	*/
	FServerSideRewindResult ProjectileServerSideRewind(
		ASarCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(
		ASarCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime
	);

	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(
		ASarCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);
	
protected:
	virtual void BeginPlay() override;
	void SaveFramePackage(FFramePackage& Package);
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);

	void CacheBoxPositions(ASarCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(ASarCharacter* HitCharacter, const FFramePackage& Package);
	void ResetHitBoxes(ASarCharacter* HitCharacter, const FFramePackage& Package);
	void EnableCharacterMeshCollision(ASarCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	void SaveFramePackage();
	FFramePackage GetFrameToCheck(ASarCharacter* HitCharacter, float HitTime);

	/** 
	* Hitscan
	*/
	FServerSideRewindResult ConfirmHit(
	const FFramePackage& Package, 
	ASarCharacter* HitCharacter, 
	const FVector_NetQuantize& TraceStart, 
	const FVector_NetQuantize& HitLocation);
	
	/** 
	* Projectile
	*/
	FServerSideRewindResult ProjectileConfirmHit(
		const FFramePackage& Package,
		ASarCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);
private:
	UPROPERTY()
	ASarCharacter* Character;

	UPROPERTY()
	class ASarPlayerController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
