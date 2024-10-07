// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameState.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void ASGameState::OnRep_WaveState(EWaveState OldState)
{
	WaveStateChanged(WaveState, OldState);
	if (WaveState == EWaveState::WaveInProgress)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), SirenSound);
	}
}

void ASGameState::SetWaveState(EWaveState NewState)
{
	if (HasAuthority())
	{
		EWaveState OldState = WaveState;
		WaveState = NewState;
		// Call on server. Because OnRep functions only called on clients
		OnRep_WaveState(OldState);
	}
}

EWaveState ASGameState::GetCurrentWaveState()
{
	return WaveState;
}

int32 ASGameState::GetElapsedCounterTime()
{
	return ElapsedCounterTime;
}

void ASGameState::SetElapsedCounterTime(int32 GameModeElapsedCounterTime)
{
	ElapsedCounterTime = GameModeElapsedCounterTime;
}

int32 ASGameState::GetWaveCount()
{
	return WaveCount;
}

void ASGameState::SetWaveCount(int32 NewWaveCount)
{
	WaveCount = NewWaveCount;
}

/* Replicate object for networking*/
void ASGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASGameState, WaveState);
	DOREPLIFETIME(ASGameState, ElapsedCounterTime);
	DOREPLIFETIME(ASGameState, WaveCount);
}
