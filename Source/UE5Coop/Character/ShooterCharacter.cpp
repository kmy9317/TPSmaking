#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "UE5Coop/Components/CombatComponent.h"
#include "UE5Coop/Components/ParkourMovementComponent.h"
#include "UE5Coop/Weapons/Weapon.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetMaterialLibrary.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/InputComponent.h"

AShooterCharacter::AShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	ParkourMovement = CreateDefaultSubobject<UParkourMovementComponent>(TEXT("ParkourMovement"));
}

void AShooterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	Combat->SetShooterCharacter(this);
}

void AShooterCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	if (ParkourMovement)
	{
		ParkourMovement->MovementChanged(PrevMovementMode, GetCharacterMovement()->MovementMode.GetValue());
	}
}

void AShooterCharacter::Landed(const FHitResult& Hit)
{
	Super::OnLanded(Hit);
	//ParkourMovement->LandEvent();
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (ParkourMovement)
	{
		ParkourMovement->Initialize(this);
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		SubSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (SubSystem)
		{
			SubSystem->AddMappingContext(GroundModeContext, 0);
		}
	}
	SpawnAbilityWeapon();
	SetMaterialInstanceDynamics();
}

void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);
	SetMaterialParamters();
}

float AShooterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void AShooterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;

	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (!ShouldAO_YawBeZero()) // Standing still, not jumping, specific parkour movement
	{
		bRotateRootBone = !IsNotSpecificParkourMovement() ? false : true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		TurnInPlace(DeltaTime);

	}
	if (ShouldAO_YawBeZero()) // Running or Jumping
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = FMath::FInterpTo(AO_Yaw, 0.f, DeltaTime, 20);
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;
}

bool AShooterCharacter::ShouldAO_YawBeZero()
{
	if (ParkourMovement == nullptr) return false;
	return (CalculateSpeed() > 0.f || GetCharacterMovement()->IsFalling()) && IsNotSpecificParkourMovement();
}

bool AShooterCharacter::IsNotSpecificParkourMovement()
{
	return ParkourMovement->CurrentParkourMovementMode != EParkourMovementType::EPM_RightWallRun &&
		ParkourMovement->CurrentParkourMovementMode != EParkourMovementType::EPM_LeftWallRun &&
		ParkourMovement->CurrentParkourMovementMode != EParkourMovementType::EPM_Slide;
}

void AShooterCharacter::SetMaterialInstanceDynamics()
{
	TArray<UMaterialInterface*> Materials = GetMesh()->GetMaterials();
	
	if (Materials.IsEmpty()) return;

	for (int32 Index = 0; Index < Materials.Num(); Index++)
	{
		UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(Materials[Index]);
		if (DynamicMaterial)
		{
			MaterialInstanceDynamics.Add(DynamicMaterial);
			GetMesh()->SetMaterial(Index, Materials[Index]);
		}	
	}
}

void AShooterCharacter::SetMaterialParamters()
{
	if (MaterialInstanceDynamics.IsEmpty() || FollowCamera == nullptr) return;
	float DistanceCharacterToCamera = FVector::Distance(GetActorLocation(), FollowCamera->GetComponentLocation());
	float Opacity = UKismetMathLibrary::MapRangeClamped(DistanceCharacterToCamera, 70.f, 130.f, 0.f, 1.f);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, -1, FColor::Blue, FString::Printf(TEXT("Distance : %f"), DistanceCharacterToCamera));
	}

	for (auto Material : MaterialInstanceDynamics)
	{
		Material->SetScalarParameterValue(TEXT("Opacity"), Opacity);
	}

	// TODO EquipWeapons (including sub weapons)
	if (InitAbilityWeapon)
	{
		InitAbilityWeapon->SetMaterialParamters(Opacity);
	}
}

void AShooterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 2.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{	
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Move);
		EnhancedInputComponent->BindAction(FlyingMoveAction, ETriggerEvent::Triggered, this, &AShooterCharacter::FlyingMove);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Jump);
		EnhancedInputComponent->BindAction(SlidingAction, ETriggerEvent::Triggered, this, &AShooterCharacter::SlidingButtonPressed);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AShooterCharacter::SprintButtonPressed);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AShooterCharacter::CrouchButtonPressed);
		EnhancedInputComponent->BindAction(DisableFlyingAction, ETriggerEvent::Triggered, this, &AShooterCharacter::DisableFlyingMode);
		EnhancedInputComponent->BindAction(EquipAbilityWeaponAction, ETriggerEvent::Triggered, this, &AShooterCharacter::EquipAbilityWeaponButtonPressed);
		EnhancedInputComponent->BindAction(FireWeaponAction, ETriggerEvent::Triggered, this, &AShooterCharacter::FireButtonPressedOrReleased);
	}
}

void AShooterCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	FRotator Rotation;
	FVector ForwardDirection;
	FVector RightDirection;

	Rotation = FRotator(0.f, Controller->GetControlRotation().Yaw, 0.f);
	ForwardDirection = FVector(FRotationMatrix(Rotation).GetUnitAxis(EAxis::X));
	RightDirection = FVector(FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y));

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void AShooterCharacter::FlyingMove(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	FRotator Rotation;
	FVector ForwardDirection;
	FVector RightDirection;

	Rotation = Controller->GetControlRotation();
	ForwardDirection = FVector(FRotationMatrix(Rotation).GetUnitAxis(EAxis::X));
	RightDirection = FVector(FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y));

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void AShooterCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerPitchInput(LookAxisVector.Y);
	AddControllerYawInput(LookAxisVector.X);
}

void AShooterCharacter::Jump()
{
	if (ParkourMovement->CurrentParkourMovementMode == EParkourMovementType::EPM_None || ParkourMovement->CurrentParkourMovementMode == EParkourMovementType::EPM_Sprint)
		FlyingOrJump();

	if (PlayerStatus != EPlayerStatus::EPS_Flying) ParkourMovement->JumpEvent();
}

void AShooterCharacter::FlyingOrJump()
{
	if (GetCharacterMovement()->IsFalling())
	{
		SetupToFlyingMode();
	}
	else Super::Jump();
}

void AShooterCharacter::SlidingButtonPressed()
{
	if (Combat && Combat->CombatAction != ECombatAction::ECA_Unoccupied) return;
	if (ParkourMovement->CurrentParkourMovementMode == EParkourMovementType::EPM_Sprint)
	{
		//PlaySlidingMontage();
	}
}

void AShooterCharacter::PlaySlidingMontage()
{
	SetAnimRootMotionTranslationScale(2.5f);

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SlidingMontage)
	{
		AnimInstance->Montage_Play(SlidingMontage);
		PlayerStatus = EPlayerStatus::EPS_Sliding;
	}
}

void AShooterCharacter::FinishSliding()
{
	SetupToWalkingMode();
}

void AShooterCharacter::PlayEquipAbilityWeaponMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipAbilityWeaponMontage)
	{
		AnimInstance->Montage_Play(EquipAbilityWeaponMontage);
	}
}

void AShooterCharacter::PlayUnequipAbilityWeaponMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && UnequipAbilityWeaponMontage)
	{
		AnimInstance->Montage_Play(UnequipAbilityWeaponMontage);
	}
}

void AShooterCharacter::SprintButtonPressed()
{
	if (GetCharacterMovement()->IsFalling()) return;
	
	if (ParkourMovement)
	{
		ParkourMovement->SprintEvent();
	}
}

void AShooterCharacter::CrouchButtonPressed()
{
	if (GetCharacterMovement()->IsFalling()) return;

	ParkourMovement->CrouchSlideEvent();
}

void AShooterCharacter::EquipAbilityWeaponButtonPressed()
{
	bool bCanEquip = ParkourMovement->CurrentParkourMovementMode == EParkourMovementType::EPM_None ||
		ParkourMovement->CurrentParkourMovementMode == EParkourMovementType::EPM_Crouch;

	if (Combat && bCanEquip)
	{
		Combat->HandleAbilityWeaponButtonPressed(InitAbilityWeapon);
	}
}


void AShooterCharacter::FireButtonPressedOrReleased(const FInputActionValue& Value)
{
	const bool bFireButtonPressed = Value.Get<bool>();
	if (Combat)
	{
		Combat->FireButtonPressed(bFireButtonPressed);
	}
}

void AShooterCharacter::DisableFlyingMode()
{
	SetupToWalkingMode();
}

void AShooterCharacter::SetupToWalkingMode()
{
	if (PlayerStatus == EPlayerStatus::EPS_Flying)
	{
		SubSystem->RemoveMappingContext(FlyingModeContext);
		SubSystem->AddMappingContext(GroundModeContext, 0);
	}
	PlayerStatus = EPlayerStatus::EPS_Walking;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AShooterCharacter::SetupToFlyingMode()
{
	PlayerStatus = EPlayerStatus::EPS_Flying;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	SubSystem->RemoveMappingContext(GroundModeContext);
	SubSystem->AddMappingContext(FlyingModeContext, 0);
}

void AShooterCharacter::SpawnAbilityWeapon()
{
	UWorld* World = GetWorld();
	if (World && AbilityWeapon)
	{
		InitAbilityWeapon = World->SpawnActor<AWeapon>(AbilityWeapon);
		if (Combat && InitAbilityWeapon)
		{
			InitAbilityWeapon->SetOwner(this);
			Combat->InitAttachAbilityWeapon(InitAbilityWeapon);
		}
	}
}

EParkourMovementType AShooterCharacter::GetParkourMovementMode() const
{
	if (ParkourMovement == nullptr) return EParkourMovementType::EPM_MAX;
	return ParkourMovement->CurrentParkourMovementMode;
}

ECombatState AShooterCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}


ECombatAction AShooterCharacter::GetCombatAction() const
{
	if (Combat == nullptr) return ECombatAction::ECA_MAX;
	return Combat->CombatAction;
}

bool AShooterCharacter::GetUseControlRotationYaw() const
{
	if (Combat == nullptr) return false;
	return Combat->CombatState == ECombatState::ECS_Travel ? false : true;
}

bool AShooterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

AWeapon* AShooterCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector AShooterCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}


