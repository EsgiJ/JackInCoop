// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterWeapon.h"
#include "DrawDebugHelpers.h"
#include "JackInCoop/JackInCoop.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"


static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("JACKINCOOP.DebugWeapons"),
	DebugWeaponDrawing,
	TEXT("Draw Debug Lines for Weapons"),
	ECVF_Cheat);

// Sets default values
AShooterWeapon::AShooterWeapon()
{
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	MuzzleSocketName = "MuzzleSocket";
	TracerBeamEndName = "BeamEnd";

	BaseDamage = 20.0f;

	//RPM, bullets per minute
	RateOfFire = 600.0f;
}

void AShooterWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60.0f / RateOfFire;
}

void AShooterWeapon::Fire()
{
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		FVector ActorEyesLocation;
		FRotator ActorEyesRotation;
		
		MyOwner->GetActorEyesViewPoint(ActorEyesLocation, ActorEyesRotation);

		FVector ShotDirection = ActorEyesRotation.Vector();
		FVector LineTraceEndLocation = ActorEyesLocation + ShotDirection * 10000;

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		FVector TracerEndPoint = LineTraceEndLocation;
		
		FHitResult HitResult;
		if(GetWorld()->LineTraceSingleByChannel(HitResult, ActorEyesLocation, LineTraceEndLocation, COLLISION_WEAPON, QueryParams))
		{
			AActor* HitActor = HitResult.GetActor();

			EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());

			float ActualDamage = BaseDamage;
			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= 4.0f;
			}
			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, HitResult, MyOwner->GetInstigatorController(), this, DamageType);

			UParticleSystem* SelectedEffect = nullptr;

			switch (SurfaceType)
			{
			case SURFACE_FLESHDEFAULT:
				SelectedEffect = FleshImpactEffect;
				break;
			case SURFACE_FLESHVULNERABLE:
				SelectedEffect = FleshImpactEffect;
				break;
			default:
				SelectedEffect = DefaultImpactEffect;
				break;
			}
			if (SelectedEffect)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
			}
			if (DebugWeaponDrawing > 0)
			{
				DrawDebugLine(GetWorld(), ActorEyesLocation, LineTraceEndLocation, FColor::Black, false, 1.0f, 0, 3.0f);
			}
			TracerEndPoint = HitResult.ImpactPoint;
		}
		PlayFireEffects(TracerEndPoint);
	}
}

void AShooterWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &AShooterWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void AShooterWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void AShooterWeapon::PlayFireEffects(FVector TracerEndPoint)
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComponent, MuzzleSocketName);
	}
	if (TracerEffect)
	{
		UParticleSystemComponent* TracerComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MeshComponent->GetSocketLocation(MuzzleSocketName));
		if (TracerComponent)
		{
			TracerComponent->SetVectorParameter(TracerBeamEndName, TracerEndPoint);
		}
	}
	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
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


