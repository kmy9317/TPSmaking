// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UE5Coop/Components/StatComponent.h"
#include "CharacterStatComponent.generated.h"

/**
 * 
 */
UCLASS()
class UE5COOP_API UCharacterStatComponent : public UStatComponent
{
	GENERATED_BODY()
	
protected:

	virtual void BeginPlay() override;
	virtual bool ApplyHealthChange(AActor* InstigatorActor, float Delta) override;

	void InitStatOverlay();

	// Stamina
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
	FORCEINLINE float GetCurrentStamina() { return CurrentStamina; }
	float GetStaminaPercent();
};
