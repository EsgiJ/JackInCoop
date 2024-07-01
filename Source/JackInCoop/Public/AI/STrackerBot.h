// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"

class UHealthComponent;

UCLASS()
class JACKINCOOP_API ASTrackerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASTrackerBot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
    UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
    UHealthComponent* HealthComp;

	UFUNCTION()
	void HandleTakeDamage(UHealthComponent* OwningHealthComp, float Health, float
	HealthDelta,const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	
	FVector GetNextPathPoint();

	FVector NextPathPoint;

	UPROPERTY(EditDefaultsOnly, Category = "TracketBot")
	float MovementForce;
	
	UPROPERTY(EditDefaultsOnly, Category = "TracketBot")
	bool bUseVelocityChange;
	
	UPROPERTY(EditDefaultsOnly, Category = "TracketBot")
	float RequiredDistanceToTarget;

	//Dynamic material to pulse on damage
	UMaterialInstanceDynamic* MatInst;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	UParticleSystem* ExplosionEffect;
	
	void SelfDestruct();

	bool bExploded;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion")
	float ExplosionDamage;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
