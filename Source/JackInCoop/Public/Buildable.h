// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Buildable.generated.h"

class AShooterCharacter;
class UAnimationAsset;
class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class JACKINCOOP_API ABuildable : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABuildable();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
	UStaticMeshComponent* BuildMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
	UStaticMeshComponent* PreviewMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building")
	UBoxComponent* CollisionVolume;

	//Dynamic material
	UMaterialInstanceDynamic* MatInst;

	UFUNCTION(BlueprintCallable, Category = "Building")
	void Build();

	FVector GetCollisionVolumeSize() const;

	// Called to interact with the buildable object
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Interact(AShooterCharacter* Interactor);

	// Event that can be implemented in Blueprints
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnInteract(AShooterCharacter* Interactor);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
