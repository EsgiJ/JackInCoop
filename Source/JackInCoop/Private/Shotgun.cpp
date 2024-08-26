// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "DrawDebugHelpers.h"
#include "AI/Zombie.h"
#include "JackInCoop/JackInCoop.h"
#include "Kismet/GameplayStatics.h"
#include "JackInCoop/Public/ShooterCharacter.h"

AShotgun::AShotgun()
{
	PrimaryActorTick.bCanEverTick = true;

	MyPawn = GetPawnOwner();

	BaseDamage = 20.f;
	WeaponConfig.MaxAmmo = 60;
	WeaponConfig.InitialClips = 5;
	WeaponConfig.AmmoPerClip = 12;
	WeaponConfig.RateOfFire = 60;
	WeaponConfig.TimeBetweenShots = 60 / WeaponConfig.RateOfFire;

	SetReplicates(true);
	SetReplicateMovement(true);
}

void AShotgun::BeginPlay()
{
	Super::BeginPlay();
}

void AShotgun::Fire()
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

			/* Make noise while firing */
			MyPawn->MakePawnNoise(2.f);

			const int NumberOfShots = 5;
			for (int i = 0; i < NumberOfShots; i++)
			{
				/* Calculate shot direction */
				FVector ActorEyesLocation;
				FRotator ActorEyesRotation;
				MyPawn->GetActorEyesViewPoint(ActorEyesLocation, ActorEyesRotation);
				FVector ShotDirection = ActorEyesRotation.Vector();

				/* Add bullet spread to shot direction */
				float VerticalHalfRad = FMath::DegreesToRadians(BulletSpread * 2);
				float HorizontalHalfRad = FMath::DegreesToRadians(BulletSpread * 4);
				ShotDirection = FMath::VRandCone(ShotDirection, HorizontalHalfRad, VerticalHalfRad);

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
				if (GetWorld()->LineTraceSingleByChannel(HitResult, ActorEyesLocation, LineTraceEndLocation, COLLISION_WEAPON, QueryParams))
				{
					AActor* HitActor = HitResult.GetActor();

					//Determine surface type
					SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());

					float ActualDamage = BaseDamage;

					/* If surface type is SURFACE_HEAD(for instance headshot) multiply base damage by 5*/
					if (SurfaceType == SURFACE_FLESHHEAD)
					{
						ActualDamage *= 5.f;
					}
					else if (SurfaceType == SURFACE_FLESHDEFAULT)
					{
						ActualDamage *= 1.f;
					}
					else
					{
						ActualDamage *= 0.75f;
					}
					if (HitActor)
					{
						UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, HitResult, MyPawn->GetInstigatorController(), GetOwner(), DamageType);
					}

					PlayImpactEffects(SurfaceType, HitResult.ImpactPoint);

					TracerEndPoint = HitResult.ImpactPoint;


					ShotDirection.Normalize();
					float Angle = 0.f;
					FVector CrossProduct;
					if (HitActor)
					{
						Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(-1 * ShotDirection, HitActor->GetActorForwardVector())));
						// Sað veya sol tarafta olduðunu belirlemek için çapraz çarpýmý kullan
						CrossProduct = FVector::CrossProduct(HitActor->GetActorForwardVector(), ShotDirection);
					}
					bool bFromRight = CrossProduct.Z < 0;

					// Açýya göre yön belirleme
					FString HitDirection;
					if (Angle <= 45.0f)
					{
						HitDirection = "Front";
					}
					else if (Angle >= 135.0f)
					{
						HitDirection = "Back";
					}
					else
					{
						HitDirection = bFromRight ? "Right" : "Left";
					}
					// Hasar miktarýna göre hit reaction tipi
					FString HitType;
					if (ActualDamage < 20.0f)
					{
						HitType = "Light";
					}
					else if (ActualDamage < 40.0f)
					{
						HitType = "Med";
					}
					else
					{
						HitType = "Heavy";
					}

					// Shooter Character Hit Reaction logic
					if (Cast<AShooterCharacter>(HitActor))
					{
						PlayShooterCharacterHitReact(HitActor, HitDirection, HitType);
						UGameplayStatics::SpawnSoundAtLocation(GetWorld(), BulletImpactSound, HitActor->GetActorLocation(), HitActor->GetActorRotation());
					}
					else if (Cast<AZombie>(HitActor))
					{
						PlayZombieHitReact(HitActor, HitDirection, SurfaceType);
						UGameplayStatics::SpawnSoundAtLocation(GetWorld(), BulletImpactSound, HitActor->GetActorLocation(), HitActor->GetActorRotation());
					}
				}
				//DrawDebugLine(GetWorld(), ActorEyesLocation, LineTraceEndLocation, FColor::Black, false, 1.0f, 0, 3.0f);

				PlayFireEffects(TracerEndPoint);
				if (HasAuthority())
				{
					HitScanTrace.TraceTo = TracerEndPoint;
					HitScanTrace.SurfaceType = SurfaceType;
				}
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
