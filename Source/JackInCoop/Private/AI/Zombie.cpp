// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Zombie.h"

#include "Buildable.h"
#include "HealthComponent.h"
#include "ShooterCharacter.h"
#include "AI/ZombieAIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "JackInCoop/JackInCoop.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Perception/PawnSensingComponent.h"

// Sets default values
AZombie::AZombie()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->SetDefaultHealth(50.f);
	HealthComponent->TeamNum = 255;
	
	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSeningComponent"));
	PawnSensingComp->SetPeripheralVisionAngle(60.f);
	PawnSensingComp->SightRadius = 2000;
	PawnSensingComp->HearingThreshold = 600;
	PawnSensingComp->LOSHearingThreshold = 1200;
	
	/* Ignore this channel or it will absorb the trace impacts instead of the skeletal mesh */
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);
	GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f, false);
	GetCapsuleComponent()->SetCapsuleRadius(42.0f);

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetupAttachment(RootComponent);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionProfileName(TEXT("OverlapAll"));
	SphereComponent->SetGenerateOverlapEvents(true);

	LeftAttackCollisionBoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftHandAttackCollisionBoxComponent"));
	LeftAttackCollisionBoxComp->SetupAttachment(GetMesh(), FName("LeftHandCollisionSocket"));
	LeftAttackCollisionBoxComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftAttackCollisionBoxComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	LeftAttackCollisionBoxComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	LeftAttackCollisionBoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	LeftAttackCollisionBoxComp->SetBoxExtent(FVector(32.f,32.f,32.f));

	RightAttackCollisionBoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("RightHandAttackCollisionBoxComponent"));
	RightAttackCollisionBoxComp->SetupAttachment(GetMesh(), FName("RightHandCollisionSocket"));
	RightAttackCollisionBoxComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightAttackCollisionBoxComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	RightAttackCollisionBoxComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	RightAttackCollisionBoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RightAttackCollisionBoxComp->SetBoxExtent(FVector(32.f,32.f,32.f));
	
	AudioLoopComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ZombieLoopedSoundComp"));
	AudioLoopComponent->bAutoActivate = false;
	AudioLoopComponent->bAutoDestroy = false;
	AudioLoopComponent->SetupAttachment(RootComponent);

	/* Noise emitter for both players and enemies. This keeps track of MakeNoise data and is used by the pawnsensing component in our SZombieCharacter class */
	NoiseEmitterComp = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("NoiseEmitterComp"));
	
	/* These values are matched up to the CapsuleComponent above and are used to find navigation paths */
	GetMovementComponent()->NavAgentProps.AgentRadius = 42;
	GetMovementComponent()->NavAgentProps.AgentHeight = 192;
	
	/* Zombie can crouch*/
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->SetIsReplicated(true);
	
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
	CurrentZombieBehavior = EZombieBehavior::Patrolling;
	AttackDamage = 20.f;
}

// Called when the game starts or when spawned
void AZombie::BeginPlay()
{
	Super::BeginPlay();
	
	GetCharacterMovement()->MaxWalkSpeed = 600.f;

	SetupAudios();
	SetupMontages();
	BroadcastUpdateAudioLoop(false);
	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &AZombie::OnHealthChanged);
	}

	if (PawnSensingComp)
	{
		PawnSensingComp->OnSeePawn.AddDynamic(this, &AZombie::OnSeePlayer);
		PawnSensingComp->OnHearNoise.AddDynamic(this, &AZombie::OnHearNoise);
	}

	if (LeftAttackCollisionBoxComp)
	{
		LeftAttackCollisionBoxComp->OnComponentBeginOverlap.AddDynamic(this, &AZombie::OnAttackOverlap);
	}
	if (RightAttackCollisionBoxComp)
	{
		RightAttackCollisionBoxComp->OnComponentBeginOverlap.AddDynamic(this, &AZombie::OnAttackOverlap);
	}
}

void AZombie::ServerPlayAnimMontage_Implementation(UAnimMontage* AnimMontage, float InPlayRate)
{
	MulticastPlayAnimationMontage(AnimMontage, InPlayRate);
}

bool AZombie::ServerPlayAnimMontage_Validate(UAnimMontage* AnimMontage, float InPlayRate)
{
	return true;
}

// Called every frame
void AZombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/* Check if the last time we sensed a player is beyond the time out value to prevent bot from endlessly following a player. */
	if (bSensedTarget && (GetWorld()->TimeSeconds - LastSeenTime) > SenseTimeOut 
		&& (GetWorld()->TimeSeconds - LastHeardTime) > SenseTimeOut)
	{
		AZombieAIController* AIController = Cast<AZombieAIController>(GetController());
		if (AIController)
		{
			bSensedTarget = false;
			/* Reset */
			AIController->SetTargetEnemy(nullptr);
			CurrentZombieBehavior = EZombieBehavior::Patrolling;
			GetCharacterMovement()->MaxWalkSpeed = 600.f;
			/* Stop playing the hunting sound */
			BroadcastUpdateAudioLoop(false);
		}
	}
}

// Called to bind functionality to input
void AZombie::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

UHealthComponent* AZombie::GetHealthComponent() const
{
	return HealthComponent;
}

void AZombie::OnHealthChanged(UHealthComponent* HealthComp, float Health, float HealthDelta,
	const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
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
		AudioLoopComponent->Stop();
		GetMovementComponent()->StopMovementImmediately();

		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		DetachFromControllerPendingDestroy();
		
		// Ragdoll Logic
		FTimerHandle TimerHandle_Ragdoll;
		GetWorldTimerManager().SetTimer(TimerHandle_Ragdoll, this, &AZombie::ServerApplyRagdoll, 0.5f, false);
		
		SetLifeSpan(10.f);
	}
}

void AZombie::OnAttackOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this)
	{
		UGameplayStatics::ApplyDamage(OtherActor, AttackDamage, GetController(), this, UDamageType::StaticClass());
	}
}

void AZombie::AttackTarget()
{
	UAnimMontage* AttackAnimationToPlay = nullptr;
	USoundBase* AttackSoundToPlay = nullptr;
	
	int32 RandomHand = FMath::RandRange(0,1);
	FTimerDelegate TimerDel;
	if (RandomHand == 0)
	{
		if (GetCharacterMovement()->Velocity.Size() > 150.f)
		{
			if (RightHandChargedAttackMontages.Num() > 0)
			{
				int32 RandomIndex = FMath::RandRange(0, RightHandChargedAttackMontages.Num() - 1);
				AttackAnimationToPlay = RightHandChargedAttackMontages[RandomIndex];
				
				if (RandomIndex == 0)
				{
					AttackSoundToPlay = SoundAttackCharged1;
				}
				else if (RandomIndex == 1)
				{
					AttackSoundToPlay = SoundAttackCharged2;
				}
				else if (RandomIndex == 2)
				{
					AttackSoundToPlay = SoundAttackCharged3;
				}
			}
		}
		else
		{
			if (RightHandNormalAttackMontages.Num() > 0)
			{
				int32 RandomIndex = FMath::RandRange(0, RightHandNormalAttackMontages.Num() - 1);
				AttackAnimationToPlay = RightHandNormalAttackMontages[RandomIndex];
				
				AttackSoundToPlay = NormalAttackSounds[RandomIndex];
			}
		}
		TimerDel.BindUFunction(this, FName("ActivateAttackCollision"), RightAttackCollisionBoxComp);
	}
	else
	{
		if (GetCharacterMovement()->Velocity.Size() > 150.f)
		{
			if (LeftHandChargedAttackMontages.Num() > 0)
			{
				int32 RandomIndex = FMath::RandRange(0, LeftHandChargedAttackMontages.Num() - 1);
				AttackAnimationToPlay = LeftHandChargedAttackMontages[RandomIndex];

				if (RandomIndex == 0)
				{
					AttackSoundToPlay = SoundAttackCharged1;
				}
				else if (RandomIndex == 1)
				{
					AttackSoundToPlay = SoundAttackCharged2;
				}
				else if (RandomIndex == 2)
				{
					AttackSoundToPlay = SoundAttackCharged3;
				}
			}
		}
		else
		{
			if (LeftHandNormalAttackMontages.Num() > 0)
			{
				int32 RandomIndex = FMath::RandRange(0, LeftHandNormalAttackMontages.Num() - 1);
				AttackAnimationToPlay = LeftHandNormalAttackMontages[RandomIndex];
				
				AttackSoundToPlay = NormalAttackSounds[RandomIndex];
			}
		}
		TimerDel.BindUFunction(this, FName("ActivateAttackCollision"), LeftAttackCollisionBoxComp);
	}

	AZombieAIController* AIController = Cast<AZombieAIController>(GetController());
	if (AttackAnimationToPlay && AttackSoundToPlay)
	{
		if (AIController)
		{
			AIController->SetSecondsToWait(AttackAnimationToPlay->GetPlayLength());
		}
		PlayAnimMontage(AttackAnimationToPlay);
		AudioLoopComponent->Stop();
		PlayCharacterSound(AttackSoundToPlay);
		AudioLoopComponent->Play(AttackSoundToPlay->GetDuration());
	}

	PlayAnimMontage(AttackAnimationToPlay, 1.5f);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_DeactivateCollision, TimerDel, AttackAnimationToPlay->GetPlayLength()/2, false);
	if (RandomHand == 0)
	{
		DeactivateAttackCollision(RightAttackCollisionBoxComp);
	}
	else
	{
		DeactivateAttackCollision(LeftAttackCollisionBoxComp);
	}
}

void AZombie::ActivateAttackCollision(UBoxComponent* CurrentBoxComponent)
{
	if (!bHasDealtDamage)
	{
		CurrentBoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		bHasDealtDamage = true;
	}
}

void AZombie::DeactivateAttackCollision(UBoxComponent* CurrentBoxComponent)
{
	CurrentBoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	bHasDealtDamage = false; // Reset the flag for a new attack
}

void AZombie::SetupMontages()
{
	RightHandNormalAttackMontages.Add(RightHandNormalAttackAnim1);
	RightHandNormalAttackMontages.Add(RightHandNormalAttackAnim2);
	
	RightHandChargedAttackMontages.Add(RightHandChargedAttackAnim1);
	RightHandChargedAttackMontages.Add(RightHandChargedAttackAnim2);
	RightHandChargedAttackMontages.Add(RightHandChargedAttackAnim3);

	LeftHandNormalAttackMontages.Add(LeftHandNormalAttackAnim1);
	LeftHandNormalAttackMontages.Add(LeftHandNormalAttackAnim2);

	LeftHandChargedAttackMontages.Add(LeftHandChargedAttackAnim1);
	LeftHandChargedAttackMontages.Add(LeftHandChargedAttackAnim2);
	LeftHandChargedAttackMontages.Add(LeftHandChargedAttackAnim3);

	ScreamMontages.Add(ScreamAnim1);
	ScreamMontages.Add(ScreamAnim2);
	ScreamMontages.Add(ScreamAnim3);
}

void AZombie::Scream()
{
	UAnimMontage* ScreamAnimationToPlay = nullptr;
	USoundBase* ScreamSoundToPlay = nullptr;
	if (ScreamMontages.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, ScreamMontages.Num() - 1);
		
		ScreamAnimationToPlay = ScreamMontages[RandomIndex];
		ScreamSoundToPlay = ScreamSounds[RandomIndex];
		
	}
	AZombieAIController* AIController = Cast<AZombieAIController>(GetController());
	if (ScreamAnimationToPlay && ScreamSoundToPlay)
	{
		if (AIController)
		{
			AIController->SetSecondsToWait(ScreamAnimationToPlay->GetPlayLength());
		}
		PlayAnimMontage(ScreamAnimationToPlay);
		
		AudioLoopComponent->Stop();
		PlayCharacterSound(ScreamSoundToPlay);
		AudioLoopComponent->Play(ScreamSoundToPlay->GetDuration());
	}
}

void AZombie::ApplyRagdoll()
{
	if (!HasAuthority())
	{
		ServerApplyRagdoll();
	}
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
}

void AZombie::ServerApplyRagdoll_Implementation()
{
	MulticastApplyRagdoll();
}

bool AZombie::ServerApplyRagdoll_Validate()
{
	return true;
}

void AZombie::MulticastApplyRagdoll_Implementation()
{
	ApplyRagdoll();
}

void AZombie::OnSeePlayer(APawn* Pawn)
{
	if (!IsAlive())
	{
		return;
	}
	if (!bSensedTarget)
	{
		BroadcastUpdateAudioLoop(true);
	}

	CurrentZombieBehavior = EZombieBehavior::Aggressive;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	/* Keep track of the time the player was last sensed in order to clear the target */
	LastSeenTime = GetWorld()->GetTimeSeconds();
	bSensedTarget = true;

	AZombieAIController* AIController = Cast<AZombieAIController>(GetController());
	AShooterCharacter* SensedPawn = Cast<AShooterCharacter>(Pawn);
	if (AIController && SensedPawn->IsAlive())
	{
		AIController->SetTargetEnemy(SensedPawn);
	}
}

void AZombie::OnHearNoise(APawn* PawnInstigator, const FVector& Location, float Volume)
{
	if (!IsAlive())
	{
		return;
	}

	if (!bSensedTarget)
	{
		BroadcastUpdateAudioLoop(true);
	}
	CurrentZombieBehavior = EZombieBehavior::Aggressive;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;

	bSensedTarget = true;
	LastHeardTime = GetWorld()->GetTimeSeconds();

	AZombieAIController* AIController = Cast<AZombieAIController>(GetController());
	if (AIController)
	{
		AIController->SetTargetEnemy(PawnInstigator);
	}
}

bool AZombie::FindNearbyWall()
{
	FVector ActorLocation = GetActorLocation();

	TArray<AActor*> OverlappingActors;
	SphereComponent->GetOverlappingActors(OverlappingActors);

	ABuildable* NearestWall = nullptr;
	float NearestDistanceSquared = TNumericLimits<float>::Max(); // En yakýn mesafeyi baþlatýyoruz (sýfýrdan büyük bir deðerle)

	for (AActor* Actor : OverlappingActors)
	{
		// ABuildable türünde olup olmadýðýný kontrol ediyoruz
		ABuildable* Buildable = Cast<ABuildable>(Actor);
		if (Buildable)
		{
			float DistanceSquared = FVector::DistSquared(ActorLocation, Buildable->GetActorLocation());
			if (DistanceSquared < NearestDistanceSquared)
			{
				NearestDistanceSquared = DistanceSquared;
				NearestWall = Buildable;
			}
		}
	}

	AZombieAIController* AIController = Cast<AZombieAIController>(GetController());
	if (AIController && NearestWall)
	{
		AIController->SetTargetWall(NearestWall);
		return true;
	}
	return false;
}

void AZombie::SetupAudios()
{
	ScreamSounds.Add(SoundScream1);
	ScreamSounds.Add(SoundScream2);
	ScreamSounds.Add(SoundScream3);

	NormalAttackSounds.Add(SoundAttackNormal1);
	NormalAttackSounds.Add(SoundAttackNormal2);
}

UAudioComponent* AZombie::PlayCharacterSound(USoundBase* CueToPlay)
{
	if (CueToPlay)
	{
		return UGameplayStatics::SpawnSoundAttached(CueToPlay, RootComponent, NAME_None, FVector::ZeroVector, EAttachLocation::SnapToTarget, true);
	}

	return nullptr;
}

void AZombie::MulticastPlayAnimationMontage_Implementation(UAnimMontage* AnimMontage, float InPlayRate)
{
	PlayAnimMontage(AnimMontage, InPlayRate);
}

void AZombie::BroadcastUpdateAudioLoop_Implementation(bool bNewSensedTarget)
{
	/* Start playing the hunting sound and the "noticed player" sound if the state is about to change */
	if (bNewSensedTarget && !bSensedTarget)
	{
		PlayCharacterSound(SoundPlayerNoticed);

		AudioLoopComponent->SetSound(SoundHunting);
		AudioLoopComponent->Play();
	}
	else
	{
		if (ZombieBehaviorType == EZombieBehavior::Patrolling)
		{
			AudioLoopComponent->SetSound(SoundWandering);
			AudioLoopComponent->Play();
		}
		else
		{
			AudioLoopComponent->SetSound(SoundIdle);
			AudioLoopComponent->Play();
		}
	}
}

void AZombie::SetZombieBehavior(EZombieBehavior NewZombieBehaviorType)
{
	ZombieBehaviorType = NewZombieBehaviorType;
	
	AZombieAIController* AIController = Cast<AZombieAIController>(GetController());
	if (AIController)
	{
		AIController->SetBlackboardZombieBehaviorType(NewZombieBehaviorType);
	}

	BroadcastUpdateAudioLoop(bSensedTarget);
}

bool AZombie::IsAlive() const
{
	return HealthComponent->GetHealth() > 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* SERVER */

/* Replicate object for networking*/
void AZombie::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AZombie, bDied);
	DOREPLIFETIME(AZombie, CurrentZombieBehavior);
}

