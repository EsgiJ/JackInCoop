// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "ShooterCharacter.generated.h"

class ABuildable;
class APistol;
class UInventoryComponent;
class UHealthComponent;
class UInputAction;
class AShooterWeapon;
class USpringArmComponent;
class UCameraComponent;
class UInputComponent;
class UInputMappingContext;
class UBuildManagerComponent;
class UUserWidget;

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

	/* Move Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/* Look Action */
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

	/* Build Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ToggleBuildModeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RequestBuildAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* OpenSwitchWeaponWheel;
	
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPawnNoiseEmitterComponent* NoiseEmitterComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UBuildManagerComponent* BuildManager;

protected:
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
	FName PrimaryWeaponAttachSocketName;
	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
	FName SecondaryWeaponAttachSocketName;
	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
	FName PistolWeaponAttachSocketName;
	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
	FName HandPistolSocketName;
	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
	FName HandRifleSocketName;
	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
	FName HandShotgunSocketName;

	FName GetCurrentWeaponSocketName() const;
	
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float ZoomedWalkSpeed;

	/* current weapon type*/
	UPROPERTY(Replicated, BlueprintReadOnly)
	EWeaponType CurrentWeaponType;
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* HEALTH */
	
	UFUNCTION()
	void OnHealthChanged(UHealthComponent* HealthComp, float Health, float
	HealthDelta,const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	/* Pawn recently died*/
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Health")
	bool bDied;

	void ApplyRagdoll();
	
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
	
	bool CanSwitchWeapon() const;

	bool bFlashlightOn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> WeaponWheelWidgetClass;

	// Widget Blueprint'in referansý
	UPROPERTY()
	UUserWidget* WeaponWheelWidget;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* ANIMS */
	UPROPERTY(EditDefaultsOnly, Category = "Anims")
	UAnimMontage* SwitchWeaponAnim;

	float LastNoiseLoudness;

	float LastMakeNoiseTime;


public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void ChechForInteractable();

	// Called when the player presses the interaction button
	void Interact();

	// The interactable object the player is currently looking at
	UPROPERTY()
	ABuildable* CurrentInteractable;

	UPROPERTY(EditDefaultsOnly, Category = "Interactable")
	float InteractionDistance = 1000.f;
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

	UFUNCTION(BlueprintCallable)
	void SwitchToPrimaryWeapon();
	UFUNCTION(BlueprintCallable)
	void SwitchToSecondaryWeapon();
	UFUNCTION(BlueprintCallable)
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
	void SwitchWeapon(int32 WeaponIndex);
	UFUNCTION()
	void FinishWeaponSwitch(int32 WeaponIndex);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSwitchWeapon(int32 WeaponIndex);
	void ServerSwitchWeapon_Implementation(int32 WeaponIndex);
	bool ServerSwitchWeapon_Validate(int32 WeaponIndex);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayAnimationMontage(UAnimMontage* AnimMontage, float InPlayRate);
	UFUNCTION(Server,Reliable,WithValidation)
	void ServerPlayAnimMontage(UAnimMontage* AnimMontage, float InPlayRate);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerApplyRagdoll();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastApplyRagdoll();
	
	/* Reload Weapon */
	void StartReload(const FInputActionValue& Value);

	void ToggleBuildMode();
	void RequestBuild();

	void OpenSwtichWeaponWheel();
	void CloseSwitchWeaponWheel();
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
	
	bool GetWantsZoom() const;

	UHealthComponent* GetHealthComponent() const;

	bool IsAlive() const;

	/* MakeNoise hook to trigger AI noise emitting (Loudness between 0.0-1.0)  */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void MakePawnNoise(float Loudness);
	
	UFUNCTION(BlueprintCallable, Category = "AI")
	float GetLastNoiseLoudness();

	UFUNCTION(BlueprintCallable, Category = "AI")
	float GetLastMakeNoiseTime();
};
