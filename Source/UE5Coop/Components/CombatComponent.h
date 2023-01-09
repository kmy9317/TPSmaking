// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UE5Coop/ShooterTypes/CombatState.h"
#include "UE5Coop/ShooterTypes/CombatAction.h"
#include "UE5Coop/HUD/ShooterHUD.h"
#include "CombatComponent.generated.h"

class AShooterCharacter;
class AWeapon;
class AShooterHUD;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UE5COOP_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()
	

public:	
	friend class AShooterCharacter;

	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
protected:
	virtual void BeginPlay() override;

	void SetHUDCrosshairs(float DeltaTime);

	void InitAttachAbilityWeapon(AWeapon* AbilityWeapon);
	void AttachWeaponToAWSocket(AWeapon* AbilityWeapon);
	void AttachWeaponToRightHand(AWeapon* Weapon);
	void SetupCombatState(ECombatState State);

	void HandleAbilityWeaponButtonPressed(AWeapon* AbilityWeapon);
	void Equip(AWeapon* Weapon);
	void Unequip();

	UFUNCTION(BlueprintCallable)
	void AttachAbilityWeapon();

	UFUNCTION(BlueprintCallable)
	void FinishEquipAbilityWeapon();

	UFUNCTION(BlueprintCallable)
	void AttachAbilityWeaponToBackpack();

	UFUNCTION(BlueprintCallable)
	void FinishUnequipAbilityWeapon();

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);
	void FireButtonPressed(bool bPressed);
	void Fire();
	bool CanFire();
	void StartFireTimer();
	void FireTimerFinished();

private:	
	UPROPERTY()
	AShooterCharacter* ShooterCharacter;
	
	UPROPERTY(VisibleAnywhere)
	ECombatState CombatState = ECombatState::ECS_Travel;

	UPROPERTY(VisibleAnywhere)
	ECombatAction CombatAction = ECombatAction::ECA_Unoccupied;

	UPROPERTY(VisibleAnywhere)
	AWeapon* EquippedWeapon;

	UPROPERTY()
	APlayerController* PlayerController;

	UPROPERTY()
	AShooterHUD* HUD;

	FHUDPackage HUDPackage;
	bool bSwapping = false;
	FVector HitTarget;

	UPROPERTY(EditAnywhere, Category = "Trace")
	float TraceLength = 80000.f;

	bool bFireButtonPressed = false;
	FTimerHandle FireTimer;
	bool bCanFire = true;

public:
	FORCEINLINE void SetShooterCharacter(AShooterCharacter* Shooter) { ShooterCharacter = Shooter; }
};
