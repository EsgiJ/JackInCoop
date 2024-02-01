// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "ShooterCharacter.generated.h"

class UHealthComponent;
class UInputAction;
class AShooterWeapon;
class USpringArmComponent;
class UCameraComponent;
class UInputComponent;
class UInputMappingContext;

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
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* WEAPON */
	
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	bool bWantsToZoom;
    
	UPROPERTY(Replicated,BlueprintReadOnly, Category = "Weapon")
	AShooterWeapon* CurrentWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AShooterWeapon>StarterWeaponClass;

	UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
	FName WeaponAttachSocketName;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* HEALTH */
	
	UFUNCTION()
	void OnHealthChanged(UHealthComponent* HealthComp, float Health, float
	HealthDelta,const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	/* Pawn recently died*/
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Health")
	bool bDied;

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* INPUT METHODS */
	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);
	
	void BeginCrouch(const FInputActionValue& Value);
	void EndCrouch(const FInputActionValue& Value);

	void BeginZoom(const FInputActionValue& Value);
	void EndZoom(const FInputActionValue& Value);

	void StartFire(const FInputActionValue& Value);
	void StopFire(const FInputActionValue& Value);

	void StartReload(const FInputActionValue& Value);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;

	bool GetWantsZoom();
};
