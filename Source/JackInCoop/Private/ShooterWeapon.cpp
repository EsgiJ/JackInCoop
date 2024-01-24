// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterWeapon.h"
#include "DrawDebugHelpers.h"
#include "ShooterCharacter.h"
#include "JackInCoop/JackInCoop.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
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
	BulletSpread = 2.0f;

	CurrentState = EWeaponState::Idle;

	HitReactAnimArray.Add(HitReactAnim_1);
	HitReactAnimArray.Add(HitReactAnim_2);
	HitReactAnimArray.Add(HitReactAnim_3);
	HitReactAnimArray.Add(HitReactAnim_4);
	
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
	
	MyPawn = GetPawnOwner();
}

void AShooterWeapon::Fire()
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

			FVector ShotDirection = ActorEyesRotation.Vector();

			/* bullet spread*/
			float HalfRad = FMath::DegreesToRadians(BulletSpread);
			ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);

			FVector LineTraceEndLocation = ActorEyesLocation + ShotDirection * 10000;

			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(this);
			QueryParams.AddIgnoredActor(MyPawn);
			QueryParams.bTraceComplex = true;
			QueryParams.bReturnPhysicalMaterial = true;

			FVector TracerEndPoint = LineTraceEndLocation;
			
			FHitResult HitResult;
			if(GetWorld()->LineTraceSingleByChannel(HitResult, ActorEyesLocation, LineTraceEndLocation, COLLISION_WEAPON, QueryParams))
			{
				AActor* HitActor = HitResult.GetActor();
				
				/* It doesn't work
				int32 RandomHitReaction = FMath::RandRange(0,3);
				if (AShooterCharacter* HitCharacter = Cast<AShooterCharacter>(HitActor))
				{
					HitCharacter->PlayAnimMontage(HitReactAnimArray[RandomHitReaction]);
				}
				*/
				
				EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());

				float ActualDamage = BaseDamage;
				if (SurfaceType == SURFACE_FLESHVULNERABLE)
				{
					ActualDamage *= 5.0f;
				}
				UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, HitResult, MyPawn->GetInstigatorController(), this, DamageType);

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

void AShooterWeapon::StartFire()
{
	if (CanFire())
	{
		CurrentState = EWeaponState::Firing;
		float FirstDelay = FMath::Max(LastFireTime + WeaponConfig.TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
		GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &AShooterWeapon::Fire, WeaponConfig.TimeBetweenShots, true, FirstDelay);
	}
	else if (CurrentAmmoInClip <= 0)
	{
		StopFire();
		StartReload();
	}
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
	if (CanFire())
	{
		CurrentState = EWeaponState::Idle;
		GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
	}
}

bool AShooterWeapon::CanFire() const
{
	bool bGotAmmoInClip = (CurrentAmmoInClip > 0);
	bool bStateOKToFire = ( ( CurrentState ==  EWeaponState::Idle ) || ( CurrentState == EWeaponState::Firing) );
	return (bStateOKToFire && bGotAmmoInClip);
}

void AShooterWeapon::UseAmmo()
{
	CurrentAmmoInClip--;
	CurrentAmmo--;
}

void AShooterWeapon::ReloadWeapon()
{
	UGameplayStatics::SpawnSoundAttached(ReloadSound, MeshComponent, MuzzleSocketName);
	GetWorldTimerManager().SetTimer(TimerHandle_ReloadComplete, this, &AShooterWeapon::FinishReload, ReloadAnim->GetPlayLength(), false);
}


void AShooterWeapon::StartReload()
{
	if (CanReload())
	{
		if (ReloadAnim)
		{
			CurrentState = EWeaponState::Reloading;
			float AnimDuration = MyPawn->PlayAnimMontage(ReloadAnim, 1.3f,NAME_None);
			GetWorldTimerManager().SetTimer(TimerHandle_ReloadWeapon, this,  &AShooterWeapon::ReloadWeapon, AnimDuration, false, 0.0f);
		}
	}
}

void AShooterWeapon::FinishReload()
{
	if (CurrentState == EWeaponState::Reloading)
	{
		int32 ClipDelta = FMath::Min(WeaponConfig.AmmoPerClip - CurrentAmmoInClip, CurrentAmmo - CurrentAmmoInClip);
		CurrentAmmoInClip += ClipDelta;
		CurrentState = EWeaponState::Idle;
	}
}

bool AShooterWeapon::CanReload() const
{
	bool bGotAmmo = (CurrentAmmoInClip < WeaponConfig.AmmoPerClip) && (CurrentAmmo - CurrentAmmoInClip > 0);
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

