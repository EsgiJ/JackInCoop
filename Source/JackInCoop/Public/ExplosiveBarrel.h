// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExplosiveBarrel.generated.h"

class UCapsuleComponent;
class URadialForceComponent;
class UHealthComponent;

UCLASS()
class JACKINCOOP_API AExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExplosiveBarrel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* COMPONENTS */

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UCapsuleComponent* CapsuleComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	URadialForceComponent* RadialForceComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UHealthComponent* HealthComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Material")
	UMaterial* ExplodedBarrelMaterial;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* HEALTH */
	
	UFUNCTION()
	void OnHealthChanged(UHealthComponent* HealthComp, float Health, float
	HealthDelta,const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	/* Barrel recently exploded*/
	UPROPERTY(ReplicatedUsing=OnRep_Exploded, BlueprintReadOnly, Category = "Health")
	bool bExploded = false;

	UFUNCTION()
	void OnRep_Exploded();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* FX */
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UParticleSystem* ExplodeEffect;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* SOUND */

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* ExplosionSound;
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* EXPLOSIVE BARREL PARAMETERS */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category = "Explosion")
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category = "Explosion")
	float ImpulseStrength;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category = "Explosion")
	float ForceStrength;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion")
	float ExplosionDamage;
};
