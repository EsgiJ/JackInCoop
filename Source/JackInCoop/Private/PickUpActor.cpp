// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUpActor.h"

#include "PowerupActor.h"
#include "ShooterCharacter.h"
#include "Components/DecalComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
APickUpActor::APickUpActor()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(75.f);
	RootComponent = SphereComp;
	
	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	DecalComp->SetRelativeRotation(FRotator(90.f,0.f,0.f));
	DecalComp->DecalSize = FVector(64.f, 75.f, 75.f);
	DecalComp->SetupAttachment(RootComponent);

	CoolDownDuration = 10.f;

	SetReplicates(true);
}

// Called when the game starts or when spawned
void APickUpActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		ReSpawn();
	}
}

void APickUpActor::ReSpawn()
{
	UE_LOG(LogTemp, Warning, TEXT("Respawn triggered!"));
	if (PowerUpClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PowerUpClass is nullpt in %s. Please update your blueprint"), *GetName());
		return;
	}
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	PowerUpInstance = GetWorld()->SpawnActor<APowerupActor>(PowerUpClass, GetTransform(), SpawnParams);
}

void APickUpActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	AShooterCharacter* PlayerPawn = Cast<AShooterCharacter>(OtherActor);

	if (HasAuthority() && PowerUpInstance && PlayerPawn)
	{
		PowerUpInstance->ActivatePowerup(OtherActor);
		PowerUpInstance = nullptr;
		//Set Timer to respawn
		GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &APickUpActor::ReSpawn, CoolDownDuration);
	}
	
		
}


