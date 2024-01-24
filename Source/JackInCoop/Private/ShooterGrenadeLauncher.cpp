// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterGrenadeLauncher.h"
#include "ShooterCharacter.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

void AShooterGrenadeLauncher::BeginPlay()
{
	Super::BeginPlay();
	MyPawn = GetPawnOwner();

	WeaponConfig.TimeBetweenShots = 2.0f;
}

void AShooterGrenadeLauncher::Fire()
{
	if (MyPawn)
	{
		if (CanFire())
		{
			if (HipFireAnim && !MyPawn->GetWantsZoom())
			{
				MyPawn->PlayAnimMontage(HipFireAnim);
			}
			else if (IronsightsFireAnim && MyPawn->GetWantsZoom())
			{
				MyPawn->PlayAnimMontage(IronsightsFireAnim);
			}
			
			UseAmmo();
			
			FVector ActorEyesLocation;
			FRotator ActorEyesRotation;
			
			MyPawn->GetActorEyesViewPoint(ActorEyesLocation, ActorEyesRotation);
			
			FActorSpawnParameters ActorSpawnParameters;
			ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
			ActorSpawnParameters.Instigator = MyPawn->GetInstigator();

			FVector MuzzleLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);
			FRotator MuzzleRotation = MeshComponent->GetSocketRotation(MuzzleSocketName);
			GetWorld()->SpawnActor<AActor>(GrenadeProjectile, MuzzleLocation, ActorEyesRotation, ActorSpawnParameters);

			if (MuzzleEffect)
			{
				UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComponent, MuzzleSocketName);
			}
			PlayFireEffects(FVector::ZeroVector);

			LastFireTime = GetWorld()->TimeSeconds;
		}
		else
		{
			StartReload();
			StopFire();
			GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
		}
	}
}

void AShooterGrenadeLauncher::PlayFireEffects(FVector TracerEndPoint)
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComponent, MuzzleSocketName);
	}
	if (MyPawn)
	{
		APlayerController* PC = Cast<APlayerController>(MyPawn->GetController());
		if (PC)
		{
			PC->ClientStartCameraShake(FireCameraShake);
		}
	}
	if (RifleFireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), RifleFireSound, this->GetActorLocation(), this->GetActorRotation());
	}
}