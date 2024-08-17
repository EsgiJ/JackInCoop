// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterWeapon.generated.h"

class USpotLightComponent;
class USkeletalMeshComponent;

UENUM(BlueprintType)
enum class EWeaponState: uint8
{
		Idle,
		Firing,
		Reloading,
		Equipping,
		Switching
};

USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()
public:

	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY()
	FVector_NetQuantize TraceTo;
};

USTRUCT(BlueprintType)
struct FWeaponData
{
	GENERATED_BODY()
	/* max ammo */
	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 MaxAmmo;

	/* clip size */
	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 AmmoPerClip;
	/* initial clips */
	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 InitialClips;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RateOfFire;

	// Derived from RateOfFire
	float TimeBetweenShots;

	FWeaponData()
	{
		MaxAmmo = 100;
		AmmoPerClip = 20;
		InitialClips = 4;
		RateOfFire = 600;
		TimeBetweenShots = 60 / RateOfFire;
	}
};
UCLASS()
class JACKINCOOP_API AShooterWeapon : public AActor
{
	GENERATED_BODY()
	
	
public:	
	// Sets default values for this actor's properties
	AShooterWeapon();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* METHODS */
	virtual void StartFire();
	
	void StopFire();

	bool CanFire() const;
	
	void UseAmmo();

	void ReloadWeapon();

	void StartReload();

	void FinishReload();

	void FlashlightOnOff(bool bEnable);

	bool CanReload() const;
	
	float GetBulletSpread();

	void SetFlashlightVisibility(bool bVisible);
	
	float SetBulletSpread(float NewBulletSpread);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void GrantAmmo();

	EWeaponState GetCurrentState() const;
	void SetCurrentState(EWeaponState NewWeaponState);

	UFUNCTION(BlueprintCallable)
	float GetAmmoSizePerClip() const;

	/** get pawn owner */
	UFUNCTION(BlueprintCallable, Category="Game|Weapon")
	class AShooterCharacter* GetPawnOwner() const;

	/** set the weapon's owning pawn */
	void SetOwningPawn(AShooterCharacter* AShooterCharacter);
	
	/* pawn owner */
	UPROPERTY()
	class AShooterCharacter* MyPawn;

	virtual void PostInitializeComponents() override;
	

protected:

	virtual void BeginPlay() override;
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* COMPONENTS*/
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComponent;

	UPROPERTY(VisibleDefaultsOnly)
	USpotLightComponent* SpotLightComp;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* WEAPON */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
	FName FlashlightSocket;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerBeamEndName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UCameraShakeBase> FireCameraShake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	/* Bullet Spread in Degrees */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin=0.0f))
	float BulletSpread;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDamage;

	UPROPERTY(Transient ,ReplicatedUsing = OnRep_FlashlightOn)
	bool bFlashlightOn;
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetFlashLightOn(bool bNewFlashLightOn);

	UFUNCTION()
	void OnRep_FlashlightOn();
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* WEAPON DATA */
	
	/* current total ammo */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	int32 CurrentAmmo;
	/* current ammo inside clip */
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	int32 CurrentAmmoInClip;

	/* current weapon state*/
	UPROPERTY(BlueprintReadOnly)
	EWeaponState CurrentState;
	
	/* weapon data */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	FWeaponData WeaponConfig;

	UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* FX */

	virtual void PlayFireEffects(FVector TracerEndPoint);

	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
	UParticleSystem* DefaultImpactEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
	UParticleSystem* FleshImpactEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
	UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
	UParticleSystem* TracerEffect;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* SOUND */

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* RifleFireSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* ReloadSound;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* EmptyClipSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* BulletImpactSound;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* TIMERS */

	FTimerHandle TimerHandle_TimeBetweenShots;

	/** Handle for efficient management of ReloadWeapon timer */
	FTimerHandle TimerHandle_ReloadWeapon;

	/** Handle for efficient management of StopReload timer */
	FTimerHandle TimerHandle_ReloadComplete;

	float LastFireTime;
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Fire();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartFire();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopFire();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* ANIMS */
	
	float PlayAnimationMontage(UAnimMontage* AnimMontage);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayAnimationMontage(UAnimMontage* AnimMontage);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerPlayAnimationMontage(UAnimMontage* AnimMontage);

	void PlayShooterCharacterHitReact(AActor* DamagedActor, const FString& HitDirection, const FString& HitType);

	void PlayZombieHitReact(AActor* DamagedActor, const FString& HitDirection, const EPhysicalSurface SurfaceType);
	
	void SetupHitReactionMontages();
	
	TArray<UAnimMontage*> FrontHitLightMontages;
	TArray<UAnimMontage*> FrontHitMediumMontages;
	TArray<UAnimMontage*> FrontDeathMontages;

	/* Front Light Hit Reactions */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* FrontHitLightMontage_01;
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* FrontHitLightMontage_02;
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* FrontHitLightMontage_03;
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* FrontHitLightMontage_04;
	
	/* Front Medium Hit Reactions */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* FrontHitMedMontage_01;
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* FrontHitMedMontage_02;
	
	/* Front Heavy Hit Reaction */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* FrontHitHeavyMontage;

	/* Back Hit Reactions */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* BackHitLightMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* BackHitMedMontage;

	/* Left Hit Reactions */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* LeftHitLightMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* LeftHitMedMontage;

	/* Right Hit Reactions */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* RightHitLightMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* RightHitMedMontage;

	/* Front Death Animation */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* FrontDeathMontage_01;
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* FrontDeathMontage_02;
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* FrontDeathMontage_03;

	/* Back Death Animation */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* BackDeathMontage;

	/* Left Death Animation */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* LeftDeathMontage;

	/* Right Death Animation */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* RightDeathMontage;

	/* Zombie Head Hit Animation */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* ZombieHeadHitReactMontage;

	/* Zombie Left Leg Hit Animation */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* ZombieLeftLegHitReactMontage;

	/* Zombie Right Leg Hit Animation */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* ZombieRightLegHitReactMontage;

	/* Zombie Left Shoulder Hit Animation */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* ZombieLeftShoulderHitReactMontage;

	/* Zombie Right Shoulder Hit Animation */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* ZombieRightShoulderHitReactMontage;

	/* Zombie Back Hit Animation */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* ZombieBackHitMontage;
	
	/* reload animation */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* ReloadAnim;

	/* fire animation */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* HipFireAnim;

	/* fire animation */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* IronsightsFireAnim;
};

