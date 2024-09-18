// Fill out your copyright notice in the Description page of Project Settings.


#include "Buildable.h"

#include "HealthComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ABuildable::ABuildable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	
	PreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
	PreviewMesh->SetupAttachment(RootComponent);

	BuildMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildMesh"));
	BuildMesh->SetupAttachment(RootComponent);
	BuildMesh->SetVisibility(false);

	CollisionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionVolume"));
	CollisionVolume->SetupAttachment(RootComponent);
	CollisionVolume->SetCollisionProfileName(TEXT("OverlapAll"));
	CollisionVolume->SetGenerateOverlapEvents(true);

	DefautHealth = 500;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->SetDefaultHealth(DefautHealth);

	GridSize = 20.f;
	bCanBuild = true;
	bDied = false;
}

void ABuildable::Build()
{
	BuildMesh->SetVisibility(true);
	CollisionVolume->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	PreviewMesh->SetVisibility(false);
}

void ABuildable::OnHealthChanged(UHealthComponent* HealthComp, float Health, float
	HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	/* If already dead return and do nothing */
	if (bDied)
	{
		return;
	}
	/* If health is less or equal to 0; stop movement of player, destroy collision,
	 * destroy mesh after 10 seconds, detach controller*/
	if (Health <= 0.0f)
	{
		bDied = true;
		CollisionVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Destroy();
	}
}

FVector ABuildable::GetCollisionVolumeSize() const
{
	return CollisionVolume->GetScaledBoxExtent();
}

void ABuildable::Interact(AShooterCharacter* Interactor)
{
	if (BuildMesh->IsVisible())
	{
		OnInteract(Interactor);
	}
}

float ABuildable::GetGridSize()
{
	return GridSize;
}

bool ABuildable::CanBuild()
{
	return bCanBuild;
}

void ABuildable::OnCollisionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor != this)
	{
		bCanBuild = false;
	}
}

void ABuildable::OnCollisionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor != this)
	{
		bCanBuild = true;
	}
}

// Called when the game starts or when spawned
void ABuildable::BeginPlay()
{
	Super::BeginPlay();
	if (CollisionVolume)
	{
		CollisionVolume->OnComponentBeginOverlap.AddDynamic(this, &ABuildable::OnCollisionBeginOverlap);
		CollisionVolume->OnComponentEndOverlap.AddDynamic(this, &ABuildable::OnCollisionEndOverlap);
	}

	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &ABuildable::OnHealthChanged);
	}
}

// Called every frame
void ABuildable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* SERVER */
/* Replicate object for networking*/
void ABuildable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABuildable, bDied);
}
