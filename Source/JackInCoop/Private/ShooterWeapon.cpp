// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterWeapon.h"
#include "DrawDebugHelpers.h"
#include "ShooterCharacter.h"
#include "JackInCoop/JackInCoop.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Net/UnrealNetwork.h"
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

	BaseDamage = 10.0f;
	BulletSpread = 1.5f;

	CurrentState = EWeaponState::Idle;

	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

void AShooterWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (WeaponConfig.InitialClips > 0)
	{
		CurrentAmmoInClip = WeaponConfig.AmmoPerClip;
		CurrentAmmo = WeaponConfig.AmmoPerClip * WeaponConfig.InitialClips;
	}
}

void AShooterWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	//MyPawn = GetPawnOwner();
	MyPawn = Cast<AShooterCharacter>(GetOwner());
}

void AShooterWeapon::Fire()
{
	/* If pawn can fire */
	if (CanFire())
	{
		if (MyPawn)
		{
			/* Play anim montages based on bWantsZoom*/
			if (HipFireAnim && !MyPawn->GetWantsZoom())
			{
				ServerPlayAnimationMontage(HipFireAnim);
			}
			else if (IronsightsFireAnim && MyPawn->GetWantsZoom())
			{
				ServerPlayAnimationMontage(IronsightsFireAnim);
			}
			/* Decrease ammo count */
			UseAmmo();

			/* Calculate shot direction */
			FVector ActorEyesLocation;
			FRotator ActorEyesRotation;
			MyPawn->GetActorEyesViewPoint(ActorEyesLocation, ActorEyesRotation);
			FVector ShotDirection = ActorEyesRotation.Vector();

			/* Add bullet spread to shot direction */
			float HalfRad = FMath::DegreesToRadians(BulletSpread);
			ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);

			FVector LineTraceEndLocation = ActorEyesLocation + ShotDirection * 10000;

			/* Set query params */
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(this);
			QueryParams.AddIgnoredActor(MyPawn);
			QueryParams.bTraceComplex = true;
			QueryParams.bReturnPhysicalMaterial = true;

			FVector TracerEndPoint = LineTraceEndLocation;
			
			FHitResult HitResult;
			EPhysicalSurface SurfaceType = SurfaceType_Default;
			/* Check whether hit result is null or not*/
			if(GetWorld()->LineTraceSingleByChannel(HitResult, ActorEyesLocation, LineTraceEndLocation, COLLISION_WEAPON, QueryParams))
			{
				AActor* HitActor = HitResult.GetActor();

				//Determine surface type
				SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());

				float ActualDamage = BaseDamage;
				
				/* If surface type is SURFACE_FLESHVULNERABLE(for instance headshot) multiply base damage by 5*/
				if (SurfaceType == SURFACE_FLESHVULNERABLE)
				{
					ActualDamage *= 5.0f;
				}
				UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, HitResult, MyPawn->GetInstigatorController(), this, DamageType);

				PlayImpactEffects(SurfaceType, HitResult.ImpactPoint);
				
				TracerEndPoint = HitResult.ImpactPoint;
				
			}
			/* On-off console command*/
			if (DebugWeaponDrawing > 0)
			{
				DrawDebugLine(GetWorld(), ActorEyesLocation, LineTraceEndLocation, FColor::Black, false, 1.0f, 0, 3.0f);
			}
			
			PlayFireEffects(TracerEndPoint);
			
			if (HasAuthority())
			{
				HitScanTrace.TraceTo = TracerEndPoint;
				HitScanTrace.SurfaceType = SurfaceType;
			}
			
			LastFireTime = GetWorld()->TimeSeconds;
		}
	}
	/* If pawn can not fire*/
	else
	{
		StartReload();
		StopFire();
		GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
	}
}

void AShooterWeapon::ServerStopFire_Implementation()
{
	StopFire();
}

bool AShooterWeapon::ServerStopFire_Validate()
{
	return true;
}

void AShooterWeapon::ServerStartFire_Implementation()
{
	StartFire();
}

bool AShooterWeapon::ServerStartFire_Validate()
{
	return true;
}

void AShooterWeapon::OnRep_HitScanTrace()
{
	//Play cosmetic FX
	PlayFireEffects(HitScanTrace.TraceTo);

	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

void AShooterWeapon::StartFire()
{
	if (!HasAuthority())
	{
		ServerStartFire();
	}
	/* If pawn can fire */
	if (CanFire())
	{
		/* Change pawn state to Firing*/
		CurrentState = EWeaponState::Firing;
		/* Set timer to determine fire rate */
		float FirstDelay = FMath::Max(LastFireTime + WeaponConfig.TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
		GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &AShooterWeapon::Fire, WeaponConfig.TimeBetweenShots, true, FirstDelay);
	}
	/* If pawn can not fire and current ammo in clip is less or equal to zero, stop fire and reload */
	else if (CurrentAmmoInClip <= 0)
	{
		StopFire();
		StartReload();
	}
	/* If pawn can not fire and current total ammo is less or equal to zero play empty clip sound */
	else if (CurrentAmmo <= 0)
	{
		UGameplayStatics::SpawnSoundAttached(EmptyClipSound, MeshComponent, MuzzleSocketName);
	}
	else
	{
		StopFire();
	}
}

void AShooterWeapon::StopFire()
{
	if (!HasAuthority())
	{
		ServerStopFire();
	}
	/* If pawn can fire */
	if (CanFire())
	{
		/* Change pawn state to Idle*/
		CurrentState = EWeaponState::Idle;
		/* Clear timer */
		GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
	}
}

bool AShooterWeapon::CanFire() const
{
	/* Check fire conditions */
	/* Check whether there are bullets in clip or not */
	bool bGotAmmoInClip = (CurrentAmmoInClip > 0);
	/* Check whether is it okay to fire - Can not fire while reloading */
	bool bStateOKToFire = ( ( CurrentState ==  EWeaponState::Idle ) || ( CurrentState == EWeaponState::Firing) );
	return (bStateOKToFire && bGotAmmoInClip);
}

void AShooterWeapon::UseAmmo()
{
	/* Decrease ammo while firing */
	CurrentAmmoInClip--;
	CurrentAmmo--;
}

void AShooterWeapon::ReloadWeapon()
{
	/* Play empty reload sound */
	UGameplayStatics::SpawnSoundAttached(ReloadSound, MeshComponent, MuzzleSocketName);
	/* Set reload timer to disable multiple reload */
	GetWorldTimerManager().SetTimer(TimerHandle_ReloadComplete, this, &AShooterWeapon::FinishReload, ReloadAnim->GetPlayLength(), false);
}


void AShooterWeapon::StartReload()
{
	/* If pawn can fire */
	if (CanReload())
	{
		if (ReloadAnim)
		{
			/* Change pawn state to Relaading*/
			CurrentState = EWeaponState::Reloading;
			float AnimDuration = 1.0f;
			ServerPlayAnimationMontage(ReloadAnim);
			GetWorldTimerManager().SetTimer(TimerHandle_ReloadWeapon, this,  &AShooterWeapon::ReloadWeapon, AnimDuration, false, 0.0f);
		}
	}
}

void AShooterWeapon::FinishReload()
{
	/* Check if pawn is reloading */
	if (CurrentState == EWeaponState::Reloading)
	{
		/* Calculate how many ammo will be transferred from total current ammo to clip*/
		int32 ClipDelta = FMath::Min(WeaponConfig.AmmoPerClip - CurrentAmmoInClip, CurrentAmmo - CurrentAmmoInClip);
		CurrentAmmoInClip += ClipDelta;
		/* Change current state to Idle after reloading */
		CurrentState = EWeaponState::Idle;
	}
}

bool AShooterWeapon::CanReload() const
{
	/* Check reload conditions */
	/* Check if there is enough ammo to reload */
	bool bGotAmmo = (CurrentAmmoInClip < WeaponConfig.AmmoPerClip) && (CurrentAmmo - CurrentAmmoInClip > 0);
	/* Check if state is okay to reload - Can not reload while already reloading */
	bool bStateOKToReload = ((CurrentState == EWeaponState::Idle) || (CurrentState == EWeaponState::Firing));
	return bGotAmmo && bStateOKToReload;
}

float AShooterWeapon::GetBulletSpread()
{
	return BulletSpread;
}

float AShooterWeapon::SetBulletSpread(float NewBulletSpread)
{
	return BulletSpread = NewBulletSpread;
}

void AShooterWeapon::PlayFireEffects(FVector TracerEndPoint)
{
	/* Play weapon fire effects*/
	/* Muzzle effect */
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComponent, MuzzleSocketName);
	}

	/* Spawn tracer effect to show to trajectory of the bullet */
	if (TracerEffect)
	{
		UParticleSystemComponent* TracerComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MeshComponent->GetSocketLocation(MuzzleSocketName));
		if (TracerComponent)
		{
			TracerComponent->SetVectorParameter(TracerBeamEndName, TracerEndPoint);
		}
	}

	/* Add camera shake to provide decent game feel*/
	if (MyPawn)
	{
		APlayerController* PC = Cast<APlayerController>(MyPawn->GetController());
		if (PC)
		{
			PC->ClientStartCameraShake(FireCameraShake);
		}
	}

	/* Play fire sound while firing */
	if (RifleFireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), RifleFireSound, this->GetActorLocation(), this->GetActorRotation());
	}
}

void AShooterWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	
	UParticleSystem* SelectedEffect = nullptr;

	/* Select impact effect according to surface type */
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
	/* Spawn selected emitter */
	if (SelectedEffect)
	{
		FVector MuzzleLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}

class AShooterCharacter* AShooterWeapon::GetPawnOwner() const
{
	return MyPawn;
}

void AShooterWeapon::SetOwningPawn(AShooterCharacter* NewOwner)
{
	if (MyPawn != NewOwner)
	{
		SetInstigator(NewOwner);
		MyPawn = NewOwner;
		SetOwner(NewOwner);
	}
}

float AShooterWeapon::PlayAnimationMontage(UAnimMontage* AnimMontage)
{
	float Duration = 0.0f;
	if (MyPawn)
	{
		if (AnimMontage)
		{
			Duration = MyPawn->PlayAnimMontage(AnimMontage);
		}
	}
	return Duration;
}

void AShooterWeapon::ServerPlayAnimationMontage_Implementation(UAnimMontage* AnimMontage)
{
	MulticastPlayAnimationMontage(AnimMontage);
}

bool AShooterWeapon::ServerPlayAnimationMontage_Validate(UAnimMontage* AnimMontage)
{
	return true;
}

void AShooterWeapon::MulticastPlayAnimationMontage_Implementation(UAnimMontage* AnimMontage)
{
	PlayAnimationMontage(AnimMontage);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* SERVER */

/* Replicate object for networking*/
void AShooterWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AShooterWeapon, HitScanTrace, COND_SkipOwner);
}

