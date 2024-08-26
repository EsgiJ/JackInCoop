// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameMode.h"

#include "GameFramework/Pawn.h"
#include "EngineUtils.h"
#include "HealthComponent.h"
#include "SGameState.h"
#include "SPlayerState.h"

ASGameMode::ASGameMode()
{
	TimeBetweenWaves = 60.f;
	WaveDuration = 300.f;
	GameStateClass = ASGameState::StaticClass();
	PlayerStateClass = ASPlayerState::StaticClass();
	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.f;
}

void ASGameMode::SetWaveState(EWaveState NewState)
{
	ASGameState* GS = GetGameState<ASGameState>();
	if (ensureAlways(GS))
	{
		GS->SetWaveState(NewState);
	}
}

void ASGameMode::StartWave()
{
	WaveCount++;
	NrOfBotsToSpawn = 20 * WaveCount;

	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, TEXT("Starting Counter"));
	GetWorldTimerManager().SetTimer(TimerHandle_Counter, this, &ASGameMode::ClearCounter, WaveDuration, false, WaveDuration);
	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameMode::SpawnBotTimerElapsed, 1.f, true, 0.f);
	SetWaveState(EWaveState::WaveInProgress);
}

void ASGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);
	SetWaveState(EWaveState::WaitingToComplete);
}

void ASGameMode::PrepareForNextWave()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, TEXT("Starting Counter"));
	GetWorldTimerManager().SetTimer(TimerHandle_Counter, this, &ASGameMode::ClearCounter, TimeBetweenWaves, false, TimeBetweenWaves);
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameMode::StartWave, TimeBetweenWaves, false, TimeBetweenWaves);
	SetWaveState(EWaveState::WaitingToStart);
	RestartDeadPlayers();
}

void ASGameMode::CheckWaveState()
{
	bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);
	
	if (NrOfBotsToSpawn > 0 || bIsPreparingForWave)
	{
		return;
	}
	
	if (!IsAnyBotAlive())
	{
		SetWaveState(EWaveState::WaveComplete);
		PrepareForNextWave();
	}
}

void ASGameMode::CheckAnyPlayerAlive()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{
			APawn* MyPawn = PC->GetPawn();
			UHealthComponent* HealthComp = Cast<UHealthComponent>(MyPawn->GetComponentByClass(UHealthComponent::StaticClass()));
			if (ensure(HealthComp) && HealthComp->GetHealth() > 0.f)
			{
				// A player is still alive
				return;
			}
		}
	}
	//No player alive
	GameOver();
}

bool ASGameMode::IsAnyBotAlive()
{
	for (TActorIterator<APawn>It(GetWorld()); It; ++It)
	{
		APawn* TestPawn = *It;
		if (TestPawn == nullptr || TestPawn->IsPlayerControlled())
		{
			continue;
		}
		UHealthComponent* HealthComp = Cast<UHealthComponent>(TestPawn->GetComponentByClass(UHealthComponent::StaticClass()));
		if (HealthComp && HealthComp->GetHealth() > 0.f)
		{
			return true;
		}
	}
	return false;
}


void ASGameMode::RestartDeadPlayers()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn() == nullptr)
		{
			RestartPlayer(PC);
		}
	}
}

void ASGameMode::ClearCounter()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, TEXT("Clearing Counter"));
	GetWorldTimerManager().ClearTimer(TimerHandle_Counter);
}

void ASGameMode::GameOver()
{
	EndWave();
	SetWaveState(EWaveState::GameOver);
}

void ASGameMode::StartPlay()
{
	Super::StartPlay();
	PrepareForNextWave();
}

void ASGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckWaveState();
	CheckAnyPlayerAlive();

	GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Black, FString::Printf(TEXT("Elapsed Time %d"), GetElapsedCounterTime()));
}

int32 ASGameMode::GetElapsedCounterTime()
{
	return GetWorldTimerManager().GetTimerElapsed(TimerHandle_Counter);
}

void ASGameMode::SpawnBotTimerElapsed()
{
	SpawnNewBot();

	NrOfBotsToSpawn--;

	if (!IsAnyBotAlive() && TimerHandle_Counter.IsValid())
	{
		EndWave();
	}
}
