// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterGrenadeLauncher.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

void AShooterGrenadeLauncher::Fire()
{
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		FVector ActorEyesLocation;
		FRotator ActorEyesRotation;
		
		MyOwner->GetActorEyesViewPoint(ActorEyesLocation, ActorEyesRotation);
		
		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
		ActorSpawnParameters.Instigator = MyOwner->GetInstigator();

		FVector MuzzleLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);
		FRotator MuzzleRotation = MeshComponent->GetSocketRotation(MuzzleSocketName);
		GetWorld()->SpawnActor<AActor>(GrenadeProjectile, MuzzleLocation, ActorEyesRotation, ActorSpawnParameters);

		if (MuzzleEffect)
		{
			UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComponent, MuzzleSocketName);
		}
	}
}
