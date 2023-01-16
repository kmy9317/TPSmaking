// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UE5Coop/ShooterTypes/PlayerStatus.h"
#include "UE5Coop/ShooterTypes/CombatState.h"
#include "UE5Coop/ShooterTypes/CombatAction.h"
#include "UE5Coop/ShooterTypes/TurningInPlace.h"
#include "UE5Coop/ParkourMovementMode/ParkourMovementMode.h"
#include "InputActionValue.h"
#include "ShooterCharacter.generated.h"

class UEnhancedInputLocalPlayerSubsystem;
class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
class UAnimMontage;
class UCombatComponent;
class UParkourMovementComponent;
class UStatComponent;
class AWeapon;

UCLASS()
class UE5COOP_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AShooterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;
	virtual void Landed(const FHitResult& Hit) override;

	void PlaySlidingMontage();
	void PlayEquipAbilityWeaponMontage();
	void PlayUnequipAbilityWeaponMontage();

protected:
	virtual void BeginPlay() override;
	virtual void Jump() override;

	UPROPERTY(EditAnywhere, Category = Input)
		UInputMappingContext* GroundModeContext;

	UPROPERTY(EditAnywhere, Category = Input)
		UInputMappingContext* FlyingModeContext;

	UPROPERTY(EditAnywhere, Category = Input)
		UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = Input)
		UInputAction* FlyingMoveAction;

	UPROPERTY(EditAnywhere, Category = Input)
		UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = Input)
		UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = Input)
		UInputAction* SlidingAction;

	UPROPERTY(EditAnywhere, Category = Input)
		UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, Category = Input)
		UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, Category = Input)
		UInputAction* DisableFlyingAction;

	UPROPERTY(EditAnywhere, Category = Input)
		UInputAction* EquipAbilityWeaponAction;

	UPROPERTY(EditAnywhere, Category = Input)
		UInputAction* FireWeaponAction;

	void Move(const FInputActionValue& Value);
	void FlyingMove(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	void FlyingOrJump();
	void DisableFlyingMode();

	void SlidingButtonPressed();
	void SprintButtonPressed();
	void CrouchButtonPressed();
	void EquipAbilityWeaponButtonPressed();
	void FireButtonPressedOrReleased(const FInputActionValue& Value);

	void SetupToWalkingMode();
	void SetupToFlyingMode();

	UFUNCTION(BlueprintCallable)
		void FinishSliding();

	void SpawnAbilityWeapon();
	void AimOffset(float DeltaTime);

	bool ShouldAO_YawBeZero();
	bool IsNotSpecificParkourMovement();

	void SetMaterialInstanceDynamics();
	void SetMaterialParamters();

private:
	float CalculateSpeed();
	void TurnInPlace(float DeltaTime);

	UEnhancedInputLocalPlayerSubsystem* SubSystem;

	UPROPERTY(VisibleAnywhere)
		USkeletalMeshComponent* CharacterMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UCombatComponent* Combat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UParkourMovementComponent* ParkourMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UStatComponent* Stat;

	bool bJumpButtonClicked = false;

	EPlayerStatus PlayerStatus = EPlayerStatus::EPS_Walking;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
		UAnimMontage* SlidingMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
		UAnimMontage* EquipAbilityWeaponMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
		UAnimMontage* UnequipAbilityWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Movement)
		float WalkSpeed;

	UPROPERTY(EditAnywhere, Category = Weapon)
		TSubclassOf<AWeapon> AbilityWeapon;

	UPROPERTY(VisibleAnywhere, Category = Weapon)
		AWeapon* InitAbilityWeapon;

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	UPROPERTY(VisibleAnywhere)
		ETurningInPlace TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	UPROPERTY(VisibleAnywhere)
		bool bRotateRootBone;

	TArray<UMaterialInstanceDynamic*> MaterialInstanceDynamics;

public:
	FORCEINLINE EPlayerStatus GetPlayerStatus() const { return PlayerStatus; }
	FORCEINLINE void SetPlayerStatus(EPlayerStatus SetupPlayerStatus) { PlayerStatus = SetupPlayerStatus; }
	ECombatState GetCombatState() const;
	ECombatAction GetCombatAction() const;
	EParkourMovementType GetParkourMovementMode() const;
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	bool GetUseControlRotationYaw() const;
	bool IsWeaponEquipped();
	AWeapon* GetEquippedWeapon();
	FVector GetHitTarget() const;
	float GetCurrentStamina() const;
};