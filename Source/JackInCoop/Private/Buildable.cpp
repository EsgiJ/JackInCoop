// Fill out your copyright notice in the Description page of Project Settings.


#include "Buildable.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"

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
	CollisionVolume->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
}

void ABuildable::Build()
{
	BuildMesh->SetVisibility(true);
	CollisionVolume->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	PreviewMesh->SetVisibility(false);
}

FVector ABuildable::GetCollisionVolumeSize() const
{
	return CollisionVolume->GetScaledBoxExtent();
}

void ABuildable::Interact(AShooterCharacter* Interactor)
{
	OnInteract(Interactor);
}

// Called when the game starts or when spawned
void ABuildable::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABuildable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

