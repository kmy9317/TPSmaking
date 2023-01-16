// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UE5Coop/ParkourMovementMode//ParkourMovementMode.h"
#include "StatComponent.generated.h"

class AShooterCharacter;
class UStatOverlay;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UE5COOP_API UStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	friend class AShooterCharacter;

	UStatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	void InitStatOverlay();
	void CheckStamina(float DeltaTime);
	bool ShouldUseStamina(float& OutSpendAmount);
	void UseStamina(float Amount, float DeltaTime);
	void ReGenStamina();
	void SetTrueRegenStaminaBoolean();
	bool IsSpecificParkourAction(EParkourMovementType CurrentParkour);

private:
	UPROPERTY(VisibleAnywhere, Category = "Character")
		AShooterCharacter* ShooterCharacter;

	UPROPERTY()
		UStatOverlay* StatOverlay;

	// Health
	UPROPERTY(EditDefaultsOnly, Category = "Health")
		float MaxHp = 100.f;

	UPROPERTY(VisibleAnywhere, Category = "Health")
		float CurrentHp = 100.f;

	// Stamina
	UPROPERTY(EditDefaultsOnly, Category = "Stamina")
		float MaxStamina = 100.f;

	UPROPERTY(VisibleAnywhere, Category = "Stamina")
		float CurrentStamina = 100.f;

	UPROPERTY(EditAnywhere, Category = "Stamina")
		float VerticalrActionStamina = 20.f;

	UPROPERTY(EditAnywhere, Category = "Stamina")
		float SprintStamina = 20.f;

	FTimerHandle RegenStaminaTimer;
	bool bCanGenerateStamina = true;

	UPROPERTY(EditAnywhere, Category = "Stamina")
		float RegenStaminaTime = 2.f;

	UPROPERTY(EditAnywhere, Category = "Stamina")
		float RegenStaminaPerSecond = 20.f;

public:
	FORCEINLINE void SetShooterCharacter(AShooterCharacter* Shooter) { ShooterCharacter = Shooter; }
	float GetHealthPercent();
	float GetStaminaPercent();
};