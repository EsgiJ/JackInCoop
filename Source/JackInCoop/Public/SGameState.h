// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SGameState.generated.h"

UENUM(BlueprintType)
enum class EWaveState : uint8
{
	WaitingToStart,
	WaveInProgress,
	// No longer spawning new bots, waiting for players to kill remaining bots
	WaitingToComplete,
	WaveComplete,
	GameOver,
};

/**
 * 
 */
UCLASS()
class JACKINCOOP_API ASGameState : public AGameStateBase
{
	GENERATED_BODY()
protected:
	UFUNCTION()
	void OnRep_WaveState(const EWaveState OldState);

	UFUNCTION(BlueprintImplementableEvent, Category = "GameState")
	void WaveStateChanged(EWaveState NewState, EWaveState OldState);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WaveState, Category = "GameState")
	EWaveState WaveState;

public:

	UFUNCTION(BlueprintCallable)
	EWaveState GetCurrentWaveState();

	void SetWaveState(EWaveState NewState);

	UFUNCTION(BlueprintCallable)
	int32 GetElapsedCounterTime();

	void SetElapsedCounterTime(int32 GameModeElapsedCounterTime);

	UFUNCTION(BlueprintCallable)
	int32 GetWaveCount();

	void SetWaveCount(int32 NewWaveCount);
	UPROPERTY(Replicated)
	int32 ElapsedCounterTime;
	UPROPERTY(Replicated)
	int32 WaveCount;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* SirenSound;
};
