// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShooterWeapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class JACKINCOOP_API AShotgun : public AShooterWeapon
{
	GENERATED_BODY()

public:
	AShotgun();
protected:
	virtual void BeginPlay() override;

	virtual void Fire() override;
};
