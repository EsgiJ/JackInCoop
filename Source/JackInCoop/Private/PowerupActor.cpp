// Fill out your copyright notice in the Description page of Project Settings.


#include "PowerupActor.h"

#include "Net/UnrealNetwork.h"

// Sets default values
APowerupActor::APowerupActor()
{
	TotalNrOfTicks = 0;
	PowerupInterval = 0.f;
	bIsPowerUpActive = false;
	
	SetReplicates(true);
}

void APowerupActor::OnTickPowerup()
{
	TicksProcessed++;

	OnPowerupTicked();
	
	if (TicksProcessed >= TotalNrOfTicks)
	{
		OnExpired();

		bIsPowerUpActive = false;
		OnRep_PowerUpActive();
		
		//Delete timer
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
	}
}

void APowerupActor::OnRep_PowerUpActive()
{
	OnPowerUpStateChanged(bIsPowerUpActive);
}

void APowerupActor::ActivatePowerup(AActor* ActivateFor)
{
	OnActivated(ActivateFor);

	bIsPowerUpActive = true;

	/* On_Rep functions don't get called on server, so we have to manueally call this function.
	 * Because ActivatePowerup() only called on server */
	OnRep_PowerUpActive();
	
	if (PowerupInterval > 0.f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &APowerupActor::OnTickPowerup, PowerupInterval, true);
	}
	else
	{
		OnTickPowerup();
	}
}

/* Replicate object for networking*/
void APowerupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APowerupActor, bIsPowerUpActive);
}

