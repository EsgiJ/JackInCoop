// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HealthComponent.h"
#include "InventoryComponent.h"
#include "Pistol.h"
#include "PropertyPathHelpers.h"
#include "ShooterWeapon.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "JackInCoop/JackInCoop.h"
#include "Net/UnrealNetwork.h"



// Sets default values
AShooterCharacter::AShooterCharacter(): ShooterMappingContext(nullptr), MoveAction(nullptr), LookAction(nullptr)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	/* Create spring arm component */
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->SetupAttachment(RootComponent);

	/* Shooter character can crouch*/
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
	/*  */
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	/* Create health component */
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health Component"));
	HealthComponent->SetDefaultHealth(200.f);

	/* Create camera component */
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

	/* Set Ironsights parameters*/
	ZoomedWalkSpeed = 225.f;
	/* Set Zoomed Field Of View */
	ZoomedFOV = 65.0f;
	/* Set interp speed when switching from DefaultFOV to ZoomedFOV*/
	ZoomInterpSpeed = 20.0f;

	/* Socket name to attach the weapon */
	HandAttachSocketName = "HandWeaponSocket";
	PrimaryWeaponAttachSocketName = "PrimaryWeaponSocket";
	SecondaryWeaponAttachSocketName = "SecondaryWeaponSocket";
	PistolWeaponAttachSocketName = "PistolWeaponSocket";

	bFlashlightOn = false;
	
	GetCharacterMovement()->SetIsReplicated(true);
	
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	/* Add mapping context for inputs */
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(ShooterMappingContext, 0);
		}
	}

	/* Set default Field Of View */
	DefaultFOV = CameraComponent->FieldOfView;
	HealthComponent->OnHealthChanged.AddDynamic(this, &AShooterCharacter::OnHealthChanged);

	/* Check if player has authority to prevent multiple Current Weapon instances*/
	if (HasAuthority())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


		PistolWeapon = GetWorld()->SpawnActor<APistol>(PistolWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (PistolWeapon)
		{
			PistolWeapon->SetOwningPawn(this);
			PistolWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, PistolWeaponAttachSocketName);
			InventoryComponent->AddItem(PistolWeapon);
		}
		
		PrimaryWeapon = GetWorld()->SpawnActor<AShooterWeapon>(PrimaryWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (PrimaryWeapon)
		{
			PrimaryWeapon->SetOwningPawn(this);
			PrimaryWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, PrimaryWeaponAttachSocketName);
			InventoryComponent->AddItem(PrimaryWeapon);
		}

		SecondaryWeapon = GetWorld()->SpawnActor<AShooterWeapon>(SecondaryWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (SecondaryWeapon)
		{
			SecondaryWeapon->SetOwningPawn(this);
			SecondaryWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SecondaryWeaponAttachSocketName);
			InventoryComponent->AddItem(SecondaryWeapon);
		}
		CurrentWeaponType = EWeaponType::Unarmed;
	}
}

void AShooterCharacter::FlashlightOnOff()
{
	if (bFlashlightOn && CurrentWeapon)
	{
		CurrentWeapon->FlashlightOnOff(false);
		bFlashlightOn = false;
	}
	else if(CurrentWeapon)
	{
		bFlashlightOn = true;
		CurrentWeapon->FlashlightOnOff(true);
	}
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ControlRotationRep = GetControlRotationRep();
	
	/* Change FOV if player wants to zoom */
	float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;
	float NewFOV = FMath::FInterpTo(CameraComponent->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);
	CameraComponent->SetFieldOfView(NewFOV);
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	/* Bind input methods to InputActions */
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Move);
		
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Look);
		
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AShooterCharacter::BeginCrouch);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AShooterCharacter::EndCrouch);
		
		EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &AShooterCharacter::ServerBeginZoom);
		EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Completed, this, &AShooterCharacter::ServerEndZoom);

		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AShooterCharacter::StartFire);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AShooterCharacter::StopFire);

		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &AShooterCharacter::StartReload);

		EnhancedInputComponent->BindAction(SwitchToPistolWeaponAction, ETriggerEvent::Triggered, this, &AShooterCharacter::SwitchToPistolWeapon);
		EnhancedInputComponent->BindAction(SwitchToPrimaryWeaponAction, ETriggerEvent::Triggered, this, &AShooterCharacter::SwitchToPrimaryWeapon);
		EnhancedInputComponent->BindAction(SwitchToSecondaryWeaponAction, ETriggerEvent::Triggered, this, &AShooterCharacter::SwitchToSecondaryWeapon);

		EnhancedInputComponent->BindAction(FlashlightOnOffAction, ETriggerEvent::Completed, this, &AShooterCharacter::FlashlightOnOff);
	}
}

FRotator AShooterCharacter::GetControlRotationRep() const
{
	if (HasAuthority() || IsLocallyControlled())
	{
		if (GetController())
		{
			return GetController()->GetControlRotation();
		}
	}
	return ControlRotationRep;
}

FName AShooterCharacter::GetCurrentWeaponSocketName()
{
	if (CurrentWeapon->IsA(PistolWeaponClass))
	{
		return PistolWeaponAttachSocketName;
	}
	else if (CurrentWeapon->IsA(PrimaryWeaponClass))
	{
		return PrimaryWeaponAttachSocketName;
	}
	else if (CurrentWeapon->IsA(SecondaryWeaponClass))
	{
		return SecondaryWeaponAttachSocketName;
	}
	return PistolWeaponAttachSocketName;
}

void AShooterCharacter::OnHealthChanged(UHealthComponent* HealthComp, float Health, float
                                        HealthDelta,const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
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
		
		GetMovementComponent()->StopMovementImmediately();

		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		DetachFromControllerPendingDestroy();

		SetLifeSpan(10.f);
		CurrentWeapon->SetLifeSpan(10.f);
	}
}

/* Get pawn view location to calculate shot direction */
FVector AShooterCharacter::GetPawnViewLocation() const
{
	if (CameraComponent)
	{
		return CameraComponent->GetComponentLocation();
	}
	return Super::GetPawnViewLocation();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* INPUT */

void AShooterCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AShooterCharacter::Look(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		FVector2D LookAxisValue = Value.Get<FVector2D>();
		
		AddControllerYawInput(LookAxisValue.X);
		AddControllerPitchInput(LookAxisValue.Y);
	}
}

void AShooterCharacter::BeginCrouch(const FInputActionValue& Value)
{
	Crouch();
}

void AShooterCharacter::EndCrouch(const FInputActionValue& Value)
{
	UnCrouch();
}

void AShooterCharacter::SwitchToPistolWeapon()
{
	SwitchWeapon(0, HeldWeaponAttachSocketName);
	CurrentWeaponType = EWeaponType::Pistol;
}

void AShooterCharacter::SwitchToPrimaryWeapon()
{
	SwitchWeapon(1, HeldWeaponAttachSocketName);
	CurrentWeaponType = EWeaponType::Rifle;
}

void AShooterCharacter::SwitchToSecondaryWeapon()
{
	SwitchWeapon(2, HeldWeaponAttachSocketName);
	CurrentWeaponType = EWeaponType::Rifle;
}

void AShooterCharacter::SwitchWeapon(int32 WeaponIndex, FName HeldWeaponSocketName)
{
	if (!HasAuthority())
	{
		ServerSwitchWeapon(WeaponIndex, HeldWeaponSocketName);
	}

	if (InventoryComponent->InventoryItems.IsValidIndex(WeaponIndex) && CanSwitchWeapon())
	{
		FlashlightOnOff();
		if (CurrentWeapon)
		{
			CurrentWeapon->SetCurrentState(EWeaponState::Switching);
		}
		MulticastPlayAnimationMontage(SwitchWeaponAnim, 2.f);

		FTimerHandle TimerHandle_SwitchWeapon;
		FTimerDelegate TimerDel;
		TimerDel.BindUFunction(this, FName("FinishWeaponSwitch"), WeaponIndex, HeldWeaponSocketName);
		
		GetWorldTimerManager().SetTimer(TimerHandle_SwitchWeapon, TimerDel, SwitchWeaponAnim->GetPlayLength()/2, false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid weapon index: %d"), WeaponIndex);
	}
}

void AShooterCharacter::FinishWeaponSwitch(int32 WeaponIndex, FName HeldWeaponSocketName)
{
	if (CurrentWeapon)
	{
		CurrentWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HeldWeaponSocketName);
	}

	CurrentWeapon = InventoryComponent->InventoryItems[WeaponIndex];
	CurrentWeapon->SetOwningPawn(this);
	CurrentWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandAttachSocketName);

	CurrentWeapon->SetCurrentState(EWeaponState::Idle);
	HeldWeaponAttachSocketName = GetCurrentWeaponSocketName();
}

void AShooterCharacter::ServerSwitchWeapon_Implementation(int32 WeaponIndex, FName HeldWeaponSocketName)
{
	SwitchWeapon(WeaponIndex, HeldWeaponSocketName);
}

bool AShooterCharacter::ServerSwitchWeapon_Validate(int32 WeaponIndex, FName HeldWeaponSocketName)
{
	return true; // Optionally add more validation logic
}

void AShooterCharacter::MulticastPlayAnimationMontage_Implementation(UAnimMontage* AnimMontage, float InPlayRate)
{
	PlayAnimMontage(AnimMontage, InPlayRate);
}

bool AShooterCharacter::CanSwitchWeapon()
{
	if (!CurrentWeapon)
	{
		return true;
	}
	bool bStateOkToSwitch = (CurrentWeapon->GetCurrentState() == EWeaponState::Idle);
	return bStateOkToSwitch;
}

void AShooterCharacter::ServerBeginZoom_Implementation(const FInputActionValue& Value)
{
	MulticastBeginZoom();
}

bool AShooterCharacter::ServerBeginZoom_Validate(const FInputActionValue& Value)
{
	return true;
}

void AShooterCharacter::MulticastBeginZoom_Implementation()
{
	bWantsToZoom = true;
	if (CurrentWeapon)
	{
		/* Decrease bullet spread if player is aiming */
		CurrentWeapon->SetBulletSpread(0.5f);
		// Change character movement speed when zooming
		GetCharacterMovement()->MaxWalkSpeed = ZoomedWalkSpeed;
	}
}

void AShooterCharacter::ServerEndZoom_Implementation(const FInputActionValue& Value)
{
	MulticastEndZoom();
}

bool AShooterCharacter::ServerEndZoom_Validate(const FInputActionValue& Value)
{
	return true;
}

void AShooterCharacter::MulticastEndZoom_Implementation()
{
	bWantsToZoom = false;
	if (CurrentWeapon)
	{
		/* Increase bullet spread if player is not aiming */
		CurrentWeapon->SetBulletSpread(1.5f);
		// Reset character movement speed when not zooming
		GetCharacterMovement()->MaxWalkSpeed = GetDefault<AShooterCharacter>()->GetCharacterMovement()->MaxWalkSpeed;
	}
}

void AShooterCharacter::StartFire(const FInputActionValue& Value)
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}

void AShooterCharacter::StopFire(const FInputActionValue& Value)
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

void AShooterCharacter::StartReload(const FInputActionValue& Value)
{
	CurrentWeapon->StartReload();
}

bool AShooterCharacter::GetWantsZoom()
{
	return bWantsToZoom;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* SERVER */

/* Replicate object for networking*/
void AShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterCharacter, PrimaryWeapon);
	DOREPLIFETIME(AShooterCharacter, SecondaryWeapon);
	DOREPLIFETIME(AShooterCharacter, PistolWeapon);
	DOREPLIFETIME(AShooterCharacter, CurrentWeapon);
	DOREPLIFETIME(AShooterCharacter, bDied);
	DOREPLIFETIME(AShooterCharacter, bWantsToZoom);
	DOREPLIFETIME(AShooterCharacter, ControlRotationRep);
}