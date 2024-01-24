// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterWeapon.generated.h"

class USkeletalMeshComponent;

UENUM(BlueprintType)
enum class EWeaponState: uint8
{
		Idle,
		Firing,
		Reloading,
		Equipping
};

USTRUCT()
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
		RateOfFire = 300;
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

	virtual void PostInitializeComponents() override;
	
	void StartFire();
	
	void StopFire();

	bool CanFire() const;
	
	void UseAmmo();

	void ReloadWeapon();

	void StartReload();

	void FinishReload();

	bool CanReload() const;
	
	float GetBulletSpread();
	
	float SetBulletSpread(float NewBulletSpread);
	
	virtual void PlayFireEffects(FVector TracerEndPoint);

	/** get pawn owner */
	UFUNCTION(BlueprintCallable, Category="Game|Weapon")
	class AShooterCharacter* GetPawnOwner() const;

	/** set the weapon's owning pawn */
	void SetOwningPawn(AShooterCharacter* AShooterCharacter);

protected:

	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerBeamEndName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UCameraShakeBase> FireCameraShake;

	/* Bullet Spread in Degrees */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin=0.0f))
	float BulletSpread;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDamage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
	UParticleSystem* DefaultImpactEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
	UParticleSystem* FleshImpactEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
	UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
	UParticleSystem* TracerEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* RifleFireSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* ReloadSound;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* EmptyClipSound;
	
	FTimerHandle TimerHandle_TimeBetweenShots;

	/** Handle for efficient management of ReloadWeapon timer */
	FTimerHandle TimerHandle_ReloadWeapon;

	/** Handle for efficient management of StopReload timer */
	FTimerHandle TimerHandle_ReloadComplete;

	float LastFireTime;
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Fire();

	/* reload animation */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* ReloadAnim;

	/* fire animation */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* HipFireAnim;

	/* fire animation */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* IronsightsFireAnim;

	/* hit animations tarray */
	TArray<UAnimMontage*> HitReactAnimArray;

	/* fire animation */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* HitReactAnim_1;

	/* fire animation */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* HitReactAnim_2;

	/* fire animation */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* HitReactAnim_3;

	/* fire animation */
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* HitReactAnim_4;

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

	/* pawn owner */
	UPROPERTY()
	class AShooterCharacter* MyPawn;
private:
	bool bPendingStopFire;
};

