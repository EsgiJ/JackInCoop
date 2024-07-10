// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShooterWeapon.h"
#include "ShooterGrenadeLauncher.generated.h"

/**
 * 
 */

UCLASS()
class JACKINCOOP_API AShooterGrenadeLauncher : public AShooterWeapon
{
	GENERATED_BODY()
public:
	AShooterGrenadeLauncher();
	
	virtual void StartFire() override;
protected:
	virtual void BeginPlay() override;
	
	virtual void PlayFireEffects(FVector TracerEndPoint) override;
	
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<AActor> GrenadeProjectile;
};
