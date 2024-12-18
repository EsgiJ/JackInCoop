// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Zombie.generated.h"

class UPawnSensingComponent;
class UAIPerceptionComponent;
class UHealthComponent;
class UBoxComponent;
class USphereComponent;

UENUM(BlueprintType)
enum class EZombieBehavior: uint8
{
	Passive,
	Patrolling,
	Alerted,
	Aggressive
};

UCLASS()
class JACKINCOOP_API AZombie : public ACharacter
{
	GENERATED_BODY()

	/* Last time the player was spotted */
	float LastSeenTime;

	/* Last time the player was heard */
	float LastHeardTime;

	/* Last time we attacked something */
	float LastMeleeAttackTime;

	/* Time-out value to clear the sensed position of the player. Should be higher than Sense interval in the PawnSense component not never miss sense ticks. */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float SenseTimeOut;

	/* Resets after sense time-out to avoid unnecessary clearing of target each tick */
	bool bSensedTarget;
public:
	// Sets default values for this character's properties
	AZombie();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* COMPONENT */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPawnSensingComponent* PawnSensingComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAudioComponent* AudioLoopComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPawnNoiseEmitterComponent* NoiseEmitterComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* SphereComponent;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* HEALTH */

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Health")
	bool bDied;
	
	UFUNCTION()
	void OnHealthChanged(UHealthComponent* HealthComp, float Health, float
	HealthDelta,const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	void ApplyRagdoll();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerApplyRagdoll();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastApplyRagdoll();
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* ATTACK */
	float AttackDamage;
	bool bHasDealtDamage = false;

	FTimerHandle TimerHandle_StartLineTrace;
	FTimerHandle TimerHandle_StopLineTrace;


	FName RightArmTraceStartName;
	FName RightArmTraceEndName;

	FName LeftArmTraceStartName;
	FName LeftArmTraceEndName;

	FName ArmTraceStartName;
	FName ArmTraceEndName;

	void StartLineTrace();
	void StopLineTrace();
	UFUNCTION()
	void OnSeePlayer(APawn* Pawn);
	
	UFUNCTION()
	void OnHearNoise(APawn* PawnInstigator, const FVector& Location, float Volume);
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* AUDIO */

	TArray<USoundBase*> ScreamSounds;
	TArray<USoundBase*> NormalAttackSounds;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* SoundPlayerNoticed;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* SoundHunting;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* SoundIdle;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* SoundWandering;

	UPROPERTY(EditDefaultsOnly, Category = "Sound") 
	USoundBase* SoundAttackNormal1;
	UPROPERTY(EditDefaultsOnly, Category = "Sound") 
	USoundBase* SoundAttackNormal2;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound") 
	USoundBase* SoundAttackCharged1;
	UPROPERTY(EditDefaultsOnly, Category = "Sound") 
	USoundBase* SoundAttackCharged2;
	UPROPERTY(EditDefaultsOnly, Category = "Sound") 
	USoundBase* SoundAttackCharged3;
	
	/* Scream Sounds */
	UPROPERTY(EditDefaultsOnly, Category = "Sound") 
	USoundBase* SoundScream1;
	UPROPERTY(EditDefaultsOnly, Category = "Sound") 
	USoundBase* SoundScream2;
	UPROPERTY(EditDefaultsOnly, Category = "Sound") 
	USoundBase* SoundScream3;
	
	void SetupAudios();
	
	/* Update the vocal loop of the zombie (idle, wandering, hunting) */
	UFUNCTION(Reliable, NetMulticast)
	void BroadcastUpdateAudioLoop(bool bNewSensedTarget);

	UAudioComponent* PlayCharacterSound(USoundBase* CueToPlay);

	/* ZombieBehavior */
	UPROPERTY(Replicated, BlueprintReadOnly)
	EZombieBehavior CurrentZombieBehavior;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* ANIMS */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayAnimationMontage(UAnimMontage* AnimMontage, float InPlayRate);
	UFUNCTION(Server,Reliable,WithValidation)
	void ServerPlayAnimMontage(UAnimMontage* AnimMontage, float InPlayRate);

	void SetupMontages();
	
	TArray<UAnimMontage*> RightHandNormalAttackMontages;
	TArray<UAnimMontage*> LeftHandNormalAttackMontages;
	TArray<UAnimMontage*> RightHandChargedAttackMontages;
	TArray<UAnimMontage*> LeftHandChargedAttackMontages;
	TArray<UAnimMontage*> ScreamMontages;
	
	/* Right Hand Normal Attack Anims */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* RightHandNormalAttackAnim1;
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* RightHandNormalAttackAnim2;

	/* Left Hand Normal Attack Anims */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* LeftHandNormalAttackAnim1;
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* LeftHandNormalAttackAnim2;

	/* Right Hand Charged Attack Anims */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* RightHandChargedAttackAnim1;
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* RightHandChargedAttackAnim2;
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* RightHandChargedAttackAnim3;

	/* Left Hand Charged Attack Anims */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* LeftHandChargedAttackAnim1;
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* LeftHandChargedAttackAnim2;
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* LeftHandChargedAttackAnim3;

	/* Scream Anims */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* ScreamAnim1;
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* ScreamAnim2;
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* ScreamAnim3;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/* The bot behavior we want this bot to execute, (passive/patrol) by specifying EditAnywhere we can edit this value per-instance when placed on the map. */
	UPROPERTY(EditAnywhere, Category = "AI")
	EZombieBehavior ZombieBehaviorType;
	
	/* The thinking part of the brain, steers our zombie and makes decisions based on the data we feed it from the Blackboard */
	/* Assigned at the Character level (instead of Controller) so we may use different zombie behaviors while re-using one controller. */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	class UBehaviorTree* BehaviorTree;

	/* Change default bot type during gameplay */
	void SetZombieBehavior(EZombieBehavior NewZombieBehaviorType);
	
	bool IsAlive() const;

	UHealthComponent* GetHealthComponent() const;

	void AttackTarget();

	void Scream();

	bool FindNearbyWall();
};
