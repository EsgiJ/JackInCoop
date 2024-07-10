// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
}

// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UInventoryComponent::AddItem(AShooterWeapon* Item)
{
	if (Item)
	{
		InventoryItems.Add(Item);
	}
}

void UInventoryComponent::RemoveItem(AShooterWeapon* Item)
{
	if (Item)
	{
		InventoryItems.Remove(Item);
	}
}
