// Fill out your copyright notice in the Description page of Project Settings.


#include "ExplosiveBarrel.h"

#include "HealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AExplosiveBarrel::AExplosiveBarrel()
{
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = MeshComponent;
	
 	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComponent"));
	RadialForceComponent->SetupAttachment(MeshComponent);
	
	ExplosionRadius = 500.0f;
	ForceStrength = 1000.0f;
	ImpulseStrength = 1000.0f;
	ExplosionDamage = 150.0f;
	
	RadialForceComponent->bIgnoreOwningActor = true;
	RadialForceComponent->bImpulseVelChange = true;
	RadialForceComponent->bAutoActivate = false;
	RadialForceComponent->Radius = ExplosionRadius;
	RadialForceComponent->ForceStrength = ForceStrength;
	RadialForceComponent->ImpulseStrength = ImpulseStrength;

	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void AExplosiveBarrel::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->OnHealthChanged.AddDynamic(this, &AExplosiveBarrel::OnHealthChanged);
	
}

void AExplosiveBarrel::OnHealthChanged(UHealthComponent* HealthComp, float Health, float HealthDelta,
	const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (bExploded)
	{
		return;
	}
	if (Health <= 0.0f)
	{
		//Mark exploded
		bExploded = true;

		FVector BoostIntensity = FVector::UpVector * ImpulseStrength;
		MeshComponent->AddImpulse(BoostIntensity, NAME_None, true);

		RadialForceComponent->FireImpulse();

		//Play explosion effects on both client and server
		OnRep_Exploded();
		
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);
		
		UGameplayStatics::ApplyRadialDamage(GetWorld(), ExplosionDamage, GetActorLocation(), 
		RadialForceComponent->Radius, UDamageType::StaticClass(), IgnoredActors, DamageCauser, InstigatedBy);
	}
}

void AExplosiveBarrel::OnRep_Exploded()
{
	//Spawn explosive FX at barrel location
	UGameplayStatics::SpawnEmitterAtLocation(this, ExplodeEffect, GetActorLocation(), GetActorRotation(), FVector(5.f));
	//Play explosion sound at barrel's location
	UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation(), GetActorRotation());
	//Change material when barrel has exploded
	MeshComponent->SetMaterial(0, ExplodedBarrelMaterial);
		
	//DrawDebugSphere(GetWorld(), GetActorLocation(), RadialForceComponent->Radius, 32, FColor::Blue, false, 2.f);
}

/* Replicate object for networking*/
void AExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AExplosiveBarrel, bExploded);
}

