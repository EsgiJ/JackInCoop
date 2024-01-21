// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterWeapon.generated.h"

class USkeletalMeshComponent;

UCLASS()
class JACKINCOOP_API AShooterWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShooterWeapon();

protected:

	virtual void BeginPlay() override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerBeamEndName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UCameraShakeBase> FireCameraShake;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDamage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
	UParticleSystem* DefaultImpactEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
	UParticleSystem* FleshImpactEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
	UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
	UParticleSystem* TracerEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* RifleFireSound;

	FTimerHandle TimerHandle_TimeBetweenShots;

	float LastFireTime;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RateOfFire;

	// Derived from RateOfFire
	float TimeBetweenShots;
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Fire();
	
public:	

	void StartFire();
	
	void StopFire();
	
	void PlayFireEffects(FVector TracerEndPoint);

};
