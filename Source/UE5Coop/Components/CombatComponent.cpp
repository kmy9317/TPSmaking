// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "UE5Coop/Character/ShooterCharacter.h"
#include "UE5Coop/Weapons/Weapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UE5Coop/ParkourMovementMode//ParkourMovementMode.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ShooterCharacter)
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	PlayerController = PlayerController == nullptr ? Cast<APlayerController>(ShooterCharacter->Controller) : PlayerController;
	if (PlayerController)
	{
		HUD = HUD == nullptr ? Cast<AShooterHUD>(PlayerController->GetHUD()) : HUD;
		if (HUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairTop = EquippedWeapon->CrosshairsTop;
			}
			else
			{
				HUDPackage.CrosshairCenter = nullptr;
				HUDPackage.CrosshairLeft = nullptr;
				HUDPackage.CrosshairRight = nullptr;
				HUDPackage.CrosshairBottom = nullptr;
				HUDPackage.CrosshairTop = nullptr;
			}

			HUDPackage.CrosshairSpread =
				0.5f;

			HUDPackage.CrosshairColor = FLinearColor::White;
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::InitAttachAbilityWeapon(AWeapon* AbilityWeapon)
{
	if (AbilityWeapon)
	{
		AbilityWeapon->SetOwner(ShooterCharacter);
		AbilityWeapon->SetWeaponState(EWeaponState::EWS_Owned);
		AttachWeaponToAWSocket(AbilityWeapon);
	}
}

void UCombatComponent::AttachWeaponToAWSocket(AWeapon* AbilityWeapon)
{
	if (ShooterCharacter == nullptr || ShooterCharacter->GetMesh() == nullptr || AbilityWeapon == nullptr) return;
	const USkeletalMeshSocket* AWSocket = ShooterCharacter->GetMesh()->GetSocketByName(FName("AbilityWeaponSocket"));
	if (AWSocket)
	{
		AWSocket->AttachActor(AbilityWeapon, ShooterCharacter->GetMesh());
	}
}

void UCombatComponent::AttachWeaponToRightHand(AWeapon* Weapon)
{
	if (ShooterCharacter == nullptr || ShooterCharacter->GetMesh() == nullptr || Weapon == nullptr) return;
	const USkeletalMeshSocket* HandSocket = ShooterCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(Weapon, ShooterCharacter->GetMesh());
	}
}

void UCombatComponent::SetupCombatState(ECombatState State)
{
	if (ShooterCharacter == nullptr) return;

	CombatState = State;
	switch (CombatState)
	{
	case ECombatState::ECS_NonCombat:
		ShooterCharacter->bUseControllerRotationYaw = false;
		ShooterCharacter->GetCharacterMovement()->bOrientRotationToMovement = true;
		break;
	case ECombatState::ECS_Combat:
		ShooterCharacter->bUseControllerRotationYaw = true;
		ShooterCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		break;
	}
}

void UCombatComponent::HandleAbilityWeaponButtonPressed(AWeapon* AbilityWeapon)
{
	if (CombatAction != ECombatAction::ECA_Unoccupied) return;
	if (CombatState == ECombatState::ECS_Travel)
	{
		Equip(AbilityWeapon);
	}
	else if (CombatState == ECombatState::ECS_NonCombat || CombatState == ECombatState::ECS_Combat)
	{
		Unequip();
	}	
}

void UCombatComponent::Equip(AWeapon* Weapon)
{
	if (ShooterCharacter == nullptr) return;

	ShooterCharacter->PlayEquipAbilityWeaponMontage();
	CombatAction = ECombatAction::ECA_Swapping;
	EquippedWeapon = Weapon;

	// Temp
	CombatState = ECombatState::ECS_Combat;
	ShooterCharacter->bUseControllerRotationYaw = true;
	ShooterCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
}

void UCombatComponent::Unequip()
{
	if (ShooterCharacter == nullptr) return;

	ShooterCharacter->PlayUnequipAbilityWeaponMontage();
	CombatAction = ECombatAction::ECA_Swapping;
}

void UCombatComponent::AttachAbilityWeapon()
{
	if (EquippedWeapon)
	{
		AttachWeaponToRightHand(EquippedWeapon);
	}
}

void UCombatComponent::FinishEquipAbilityWeapon()
{
	CombatAction = ECombatAction::ECA_Unoccupied;
}

void UCombatComponent::AttachAbilityWeaponToBackpack()
{
	if (EquippedWeapon)
	{
		AttachWeaponToAWSocket(EquippedWeapon);
	}
}

void UCombatComponent::FinishUnequipAbilityWeapon()
{
	CombatAction = ECombatAction::ECA_Unoccupied;

	CombatState = ECombatState::ECS_Travel;
	EquippedWeapon = nullptr;

	// Temp
	ShooterCharacter->bUseControllerRotationYaw = false;
	ShooterCharacter->GetCharacterMovement()->bOrientRotationToMovement = true;
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D VieportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(VieportSize);
	}

	FVector2D CrosshairLocation(VieportSize.X / 2.f, VieportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		if (ShooterCharacter)
		{
			float DistanceToCharacter = (ShooterCharacter->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}

		FVector End = Start + CrosshairWorldDirection * TraceLength;
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::Fire()
{
	if (CanFire())
	{
		bCanFire = false;
		EquippedWeapon->Fire(HitTarget);
		StartFireTimer();
	}
}

bool UCombatComponent::CanFire()
{
	EParkourMovementType CurrentParkourType = ShooterCharacter->GetParkourMovementMode();

	return EquippedWeapon &&
		bCanFire &&
		CombatAction == ECombatAction::ECA_Unoccupied &&
		CurrentParkourType != EParkourMovementType::EPM_Sprint &&
		CurrentParkourType != EParkourMovementType::EPM_Mantle &&
		CurrentParkourType != EParkourMovementType::EPM_VerticalWallRun &&
		CurrentParkourType != EParkourMovementType::EPM_LedgeGrab;
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || ShooterCharacter == nullptr) return;
	ShooterCharacter->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return;
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
}
