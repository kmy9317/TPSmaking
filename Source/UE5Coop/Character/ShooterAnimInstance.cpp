// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UE5Coop/Weapons/Weapon.h"
#include "Kismet/KismetMathLibrary.h"

void UShooterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}
void UShooterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (ShooterCharacter == nullptr)
	{
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}
	if (ShooterCharacter == nullptr) return;

	FVector Velocity = ShooterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();
	ParkourMovementMode = ShooterCharacter->GetParkourMovementMode();
	TurningInPlace = ShooterCharacter->GetTurningInPlace();

	bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0 ? true : false;
	PlayerStatus = ShooterCharacter->GetPlayerStatus();
	CombatState = ShooterCharacter->GetCombatState();
	CombatAction = ShooterCharacter->GetCombatAction();
	bRotateRootBone = ShooterCharacter->ShouldRotateRootBone();
	bWeaponEquipped = ShooterCharacter->IsWeaponEquipped();
	EquippedWeapon = ShooterCharacter->GetEquippedWeapon();

	FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = ShooterCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -180.f, 180.f);

	AO_Yaw = ShooterCharacter->GetAO_Yaw();
	AO_Pitch = ShooterCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && ShooterCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		ShooterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		FTransform RightHandTransform = ShooterCharacter->GetMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
		FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(
			RightHandTransform.GetLocation(),
			RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - ShooterCharacter->GetHitTarget()));
		RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
	}

	bTransformRightHand = bUseFABRIK = CanUseTransformHand();
}

bool UShooterAnimInstance::CanUseTransformHand()
{
	return CombatState == ECombatState::ECS_Combat &&
		CombatAction != ECombatAction::ECA_Swapping &&
		ParkourMovementMode != EParkourMovementType::EPM_Sprint &&
		ParkourMovementMode != EParkourMovementType::EPM_VerticalWallRun &&
		ParkourMovementMode != EParkourMovementType::EPM_LedgeGrab &&
		ParkourMovementMode != EParkourMovementType::EPM_Mantle;
}