// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildManagerComponent.h"

#include "Buildable.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"

// Sets default values for this component's properties
UBuildManagerComponent::UBuildManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	bIsBuilding = false;
}


// Called when the game starts
void UBuildManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	PlayerCamera = GetOwner()->FindComponentByClass<UCameraComponent>();
	if (PlayerCamera == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Unable to find player camera."));
	}
}


bool UBuildManagerComponent::CheckCollision(FVector Location, FRotator Rotation, FVector Size)
{
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(CurrentBuild);
	Params.AddIgnoredActor(GetOwner());
	Params.bTraceComplex = true;

	FRotator AdjustedRotation = Rotation;
	AdjustedRotation.Yaw += 90.f;

	FCollisionShape CollisionShape = FCollisionShape::MakeBox(FVector(Size.X, Size.Y, Size.Z));
	FHitResult HitResult;

	bool bCanPlace = GetWorld()->SweepSingleByChannel(
		HitResult, 
		Location, 
		Location, 
		AdjustedRotation.Quaternion(),
		ECC_WorldStatic, 
		CollisionShape, 
		Params);
	return !bCanPlace;
}

// Called every frame
void UBuildManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bIsBuilding)
	{
		FVector Location = GetNextBuildLocation();
		FRotator Rotation = GetNextBuildRotation();

		if (CurrentBuild == nullptr)
		{
			//Create CurrentBuild
			CurrentBuild = GetWorld()->SpawnActor<ABuildable>(Buildable, Location, Rotation, FActorSpawnParameters{});
		}
		else
		{
			FRotator AdjustedRotation = CurrentBuild->GetActorRotation();
			AdjustedRotation.Yaw += 90.f;
			//DrawDebugBox(GetWorld(), CurrentBuild->PreviewMesh->GetComponentLocation(), CurrentBuild->GetCollisionVolumeSize(), AdjustedRotation.Quaternion(), FColor::Green, false, 5.0f);
			//Update location an rotation
			CurrentBuild->SetActorLocationAndRotation(Location,Rotation);
			if (!CheckCollision(CurrentBuild->PreviewMesh->GetComponentLocation(), AdjustedRotation, CurrentBuild->GetCollisionVolumeSize()))
			{
				//Pulse the material on hit
				if (CurrentBuild->MatInst == nullptr)
				{
					CurrentBuild->MatInst = CurrentBuild->PreviewMesh->CreateAndSetMaterialInstanceDynamicFromMaterial(0, CurrentBuild->PreviewMesh->GetMaterial(0));
				}

				if (CurrentBuild->MatInst)
				{
					FLinearColor NewColor = FLinearColor::Red;  
					CurrentBuild->MatInst->SetVectorParameterValue("PreviewColor", NewColor);
				}
			}
			else
			{
				//Pulse the material on hit
				if (CurrentBuild->MatInst == nullptr)
				{
					CurrentBuild->MatInst = CurrentBuild->PreviewMesh->CreateAndSetMaterialInstanceDynamicFromMaterial(0, CurrentBuild->PreviewMesh->GetMaterial(0));
				}

				if (CurrentBuild->MatInst)
				{
					FLinearColor NewColor = FLinearColor::Blue;
					CurrentBuild->MatInst->SetVectorParameterValue("PreviewColor", NewColor);
				}
			}
		}
	}
	else if (CurrentBuild != nullptr)
	{
		// Destroy CurrentBuild
		CurrentBuild->Destroy();
		CurrentBuild = nullptr;
	}
	
}

FVector UBuildManagerComponent::GetNextBuildLocation() const
{
	FVector DirectionVector = PlayerCamera->GetForwardVector() * BuildDistance;
	DirectionVector += GetOwner()->GetActorLocation();

	return FVector(
		FMath::GridSnap(DirectionVector.X, GridSize),
		FMath::GridSnap(DirectionVector.Y, GridSize),
		FloorHeight);
}

FRotator UBuildManagerComponent::GetNextBuildRotation() const
{
	FRotator Rotation = PlayerCamera->GetComponentRotation();
	return FRotator(0, FMath::GridSnap(Rotation.Yaw, 90.f), 0);
}

void UBuildManagerComponent::ToggleBuildMode()
{
	bIsBuilding = true;
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Blue, TEXT("UBuildManagerComponent::ToggleBuild()"));
	}
}

void UBuildManagerComponent::RequestBuild()
{
	if (GEngine == nullptr && (!bIsBuilding || CurrentBuild == nullptr))
	{
		return;
	}
	GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Blue, TEXT("UBuildManagerComponent::RequestBuild()"));
	if (CurrentBuild && CheckCollision(CurrentBuild->PreviewMesh->GetComponentLocation(), CurrentBuild->PreviewMesh->GetComponentRotation(),CurrentBuild->GetCollisionVolumeSize()))
	{
		CurrentBuild->Build();
		CurrentBuild = nullptr;
	}
}