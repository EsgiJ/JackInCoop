// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
AShooterWeapon::AShooterWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	MuzzleSocketName = "MuzzleSocket";
	TracerBeamEndName = "BeamEnd";
}

// Called when the game starts or when spawned
void AShooterWeapon::BeginPlay()
{
	Super::BeginPlay();
	
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

		FVector TracerEndPoint = LineTraceEndLocation;
		
		FHitResult HitResult;
		if(GetWorld()->LineTraceSingleByChannel(HitResult, ActorEyesLocation, LineTraceEndLocation, ECC_Visibility, QueryParams))
		{
			AActor* HitActor = HitResult.GetActor();
			UGameplayStatics::ApplyPointDamage(HitActor, 20.0f,ShotDirection, HitResult, MyOwner->GetInstigatorController(), this, DamageType);
			//DrawDebugLine(GetWorld(), ActorEyesLocation, LineTraceEndLocation, FColor::Black, false, 1.0f, 0, 3.0f);
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
			TracerEndPoint = HitResult.ImpactPoint;
		}
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
	}
}

// Called every frame
void AShooterWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

