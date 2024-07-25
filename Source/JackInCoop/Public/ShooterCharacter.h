// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "ShooterCharacter.generated.h"

class APistol;
class UInventoryComponent;
class UHealthComponent;
class UInputAction;
class AShooterWeapon;
class USpringArmComponent;
class UCameraComponent;
class UInputComponent;
class UInputMappingContext;

UENUM(BlueprintType)
enum class EWeaponType: uint8
{
	Unarmed,
	Rifle,
	Shotgun,
	Pistol
};

UCLASS()
class JACKINCOOP_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()
protected:
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* INPUT */
	
	/* Mapping Context */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* ShooterMappingContext;

	/* Move Move Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/* Look Look Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/* Look Crouch Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	/* Look Zoom Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ZoomAction;

	/* Look Fire Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

	/* Look Reload Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ReloadAction;

	/* Switch To Primary Weapon Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchToPrimaryWeaponAction;

	/* Switch To Secondary Weapon Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchToSecondaryWeaponAction;

	/* Switch To Secondary Weapon Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchToPistolWeaponAction;

	/* Look Reload Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FlashlightOnOffAction;
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* COMPONENTS*/
	
	/* Camera Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")												
	UCameraComponent* CameraComponent;
	
	/* Spring Arm Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* CAMERA */
	
	UPROPERTY(EditDefaultsOnly, Category="Camera")
	float ZoomedFOV;
	
	float DefaultFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (ClampMin = 0.1f, ClampMax = 100.0f))
	float ZoomInterpSpeed;

	UPROPERTY(Replicated, BlueprintReadOnly)
	FRotator  ControlRotationRep;
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* WEAPON */
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon")
	bool bWantsToZoom;

	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
	FName HandAttachSocketName;
	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
	FName PrimaryWeaponAttachSocketName;
	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
	FName SecondaryWeaponAttachSocketName;
	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
	FName PistolWeaponAttachSocketName;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float ZoomedWalkSpeed;

	/* current weapon type*/
	UPROPERTY(BlueprintReadOnly)
	EWeaponType CurrentWeaponType;
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* HEALTH */
	
	UFUNCTION()
	void OnHealthChanged(UHealthComponent* HealthComp, float Health, float
	HealthDelta,const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	/* Pawn recently died*/
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Health")
	bool bDied;
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* Inventory */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInventoryComponent* InventoryComponent;

	UPROPERTY(Replicated,BlueprintReadOnly, Category = "Inventory")
	AShooterWeapon* PrimaryWeapon;

	UPROPERTY(Replicated,BlueprintReadOnly, Category = "Inventory")
	AShooterWeapon* SecondaryWeapon;

	UPROPERTY(Replicated,BlueprintReadOnly, Category = "Inventory")
	AShooterWeapon* PistolWeapon;

	UPROPERTY(Replicated,BlueprintReadOnly, Category = "Inventory")
	AShooterWeapon* CurrentWeapon;
	
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<AShooterWeapon>PrimaryWeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<AShooterWeapon>SecondaryWeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<APistol>PistolWeaponClass;
	
	bool CanSwitchWeapon();

	bool bFlashlightOn;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* ANIMS */
	UPROPERTY(EditDefaultsOnly, Category = "Anims")
	UAnimMontage* SwitchWeaponAnim;
	
public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* INPUT METHODS */

	void FlashlightOnOff();
	
	/* Move Input */
	void Move(const FInputActionValue& Value);

	/* Look Input */
	void Look(const FInputActionValue& Value);

	/* Crouch */
	void BeginCrouch(const FInputActionValue& Value);
	void EndCrouch(const FInputActionValue& Value);

	void SwitchToPrimaryWeapon();
	void SwitchToSecondaryWeapon();
	void SwitchToPistolWeapon();
	
	/* Zoom | Ironsights | Server */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerBeginZoom(const FInputActionValue& Value);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastBeginZoom();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEndZoom(const FInputActionValue& Value);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEndZoom();

	/* Switch Weapon | Server */
	UFUNCTION()
	void SwitchWeapon(int32 WeaponIndex, FName HeldWeaponSocketName);
	UFUNCTION()
	void FinishWeaponSwitch(int32 WeaponIndex, FName HeldWeaponSocketName);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSwitchWeapon(int32 WeaponIndex, FName HeldWeaponSocketName);
	void ServerSwitchWeapon_Implementation(int32 WeaponIndex, FName HeldWeaponSocketName);
	bool ServerSwitchWeapon_Validate(int32 WeaponIndex, FName HeldWeaponSocketName);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayAnimationMontage(UAnimMontage* AnimMontage, float InPlayRate);
	
	/* Reload Weapon */
	void StartReload(const FInputActionValue& Value);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Custom function to get control rotation
	UFUNCTION(BlueprintCallable, Category = "Control")
	FRotator GetControlRotationRep() const;
	
	virtual FVector GetPawnViewLocation() const override;

	/* Fire */
	UFUNCTION(BlueprintCallable, Category = "Player")
	void StartFire(const FInputActionValue& Value);
	
	UFUNCTION(BlueprintCallable, Category = "Player")
	void StopFire(const FInputActionValue& Value);
	
	bool GetWantsZoom();
};
