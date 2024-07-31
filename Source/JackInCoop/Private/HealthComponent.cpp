// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"

#include "SGameMode.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	bIsDead = false;
	DefaultHealth = 100.0f;

	TeamNum = 255;
	
	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = DefaultHealth;
	if (GetOwner()->HasAuthority())
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage);
		}
	}
}

float UHealthComponent::GetDefaultHealth() const
{
	return DefaultHealth;
}

void UHealthComponent::SetDefaultHealth(float NewDefaultHealth)
{
	DefaultHealth = NewDefaultHealth;
}

float UHealthComponent::GetHealth() const
{
	return Health;
}

void UHealthComponent::OnRep_Health(float OldHealth)
{
	float Damage = Health - OldHealth;
	OnHealthChanged.Broadcast(this, Health, Damage , nullptr, nullptr, nullptr);
}

void UHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
                                           AController* InstigatedBy, AActor* DamageCauser)
{
	/* If damage is less or equal to zero, do not run the remaining code*/
	if (Damage <=0.0f)
	{
		return;
	}
	// Prevent friendly-fire
	if (DamageCauser != DamagedActor && IsFriendly(DamagedActor, DamageCauser))
	{
		return;
	}
	
	/* Make sure health doesn't drop below 0 */
	Health = FMath::Clamp(Health - Damage, 0.0f,DefaultHealth);

	UE_LOG(LogTemp, Log, TEXT("Health Changed: %s"), *FString::SanitizeFloat(Health));
	
	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

	if (Health <= 0.f && !bIsDead)
	{
		//DamagedActor is already dead
		bIsDead = true;
		ASGameMode* GM = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			GM->OnActorKilled.Broadcast(DamagedActor, DamageCauser, InstigatedBy);
		}
	}
}

void UHealthComponent::Heal(float HealAmount)
{
	if (HealAmount <= 0.f || Health <= 0.f)
	{
		return;
	}
	Health = FMath::Clamp(Health + HealAmount, 0.f, DefaultHealth);
	UE_LOG(LogTemp, Log, TEXT("Health Changed: %s (+%s)"), *FString::SanitizeFloat(Health),  *FString::SanitizeFloat(HealAmount));

	OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);
}

bool UHealthComponent::IsFriendly(AActor* ActorA, AActor* ActorB)
{
	// Assume friendly
	if (ActorA == nullptr || ActorB == nullptr)
	{
		return true;
	}
	
	UHealthComponent* HealthCompA = Cast<UHealthComponent>(ActorA->GetComponentByClass(UHealthComponent::StaticClass()));
	UHealthComponent* HealthCompB = Cast<UHealthComponent>(ActorB->GetComponentByClass(UHealthComponent::StaticClass()));

	// Assume friendly
	if (HealthCompA == nullptr || HealthCompB == nullptr)
	{
		return true;
	}

	return HealthCompA->TeamNum == HealthCompB->TeamNum;
}

/* Replicate object for networking*/
void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, Health);
}
