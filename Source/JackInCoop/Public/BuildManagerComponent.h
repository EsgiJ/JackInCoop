// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuildManagerComponent.generated.h"


class UCameraComponent;
class ABuildable;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JACKINCOOP_API UBuildManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBuildManagerComponent();

	UFUNCTION(BlueprintCallable)
	void ToggleBuildMode();

	UFUNCTION(BlueprintCallable)
	void RequestBuild();
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Building")
	float GridSize = 2.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Building")
	float BuildDistance = 500.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Building")
	float FloorHeight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	TSubclassOf<class ABuildable> Buildable;

	bool CheckCollision(FVector Location, FRotator Rotation, FVector Size);
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
	FVector GetNextBuildLocation() const;
	FRotator GetNextBuildRotation() const;

	bool bIsBuilding;
	UCameraComponent* PlayerCamera;
	ABuildable* CurrentBuild;
};