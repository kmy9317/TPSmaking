// Fill out your copyright notice in the Description page of Project Settings.


#include "UE5Coop/Components/CharacterStatComponent.h"
#include "UE5Coop/Character/ShooterCharacter.h"
#include "UE5Coop/HUD/StatOverlay.h"
#include "UE5Coop/HUD/ShooterHUD.h"
#include "Kismet/GameplayStatics.h"

void UCharacterStatComponent::BeginPlay()
{
	if (ShooterCharacter && ShooterCharacter->IsLocallyControlled())
	{
		InitStatOverlay();
	}
}

void UCharacterStatComponent::InitStatOverlay()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(ShooterCharacter->GetController()))
	{
		if (AShooterHUD* ShooterHUD = Cast<AShooterHUD>(PlayerController->GetHUD()))
		{
			if (UStatOverlay* ValidStatOverlay = ShooterHUD->GetStatOverlay())
			{
				StatOverlay = ValidStatOverlay;
				StatOverlay->SetHealthBarPercent(GetHealthPercent());
				StatOverlay->SetStaminaBarPercent(GetStaminaPercent());
			}
		}
	}
}

void UCharacterStatComponent::CheckStamina(float DeltaTime)
{
	if (ShooterCharacter == nullptr) return;

	float SpendAmount;
	if (ShouldUseStamina(SpendAmount))
	{
		UseStamina(SpendAmount, DeltaTime);
	}
	else if (!ShooterCharacter->GetWorldTimerManager().IsTimerActive(RegenStaminaTimer) && !bCanGenerateStamina)
	{
		ShooterCharacter->GetWorldTimerManager().SetTimer(
			RegenStaminaTimer,
			this,
			&UCharacterStatComponent::SetTrueRegenStaminaBoolean,
			RegenStaminaTime);
	}
	else if (bCanGenerateStamina)
	{
		ReGenStamina();
	}
}

bool UCharacterStatComponent::ShouldUseStamina(float& OutSpendAmount)
{
	EParkourMovementType CurrentParkour = ShooterCharacter->GetParkourMovementMode();
	if (!IsSpecificParkourAction(CurrentParkour)) return false;

	switch (CurrentParkour)
	{
	case EParkourMovementType::EPM_VerticalWallRun:
		OutSpendAmount = VerticalrActionStamina;
		break;
	case EParkourMovementType::EPM_LedgeGrab:
		OutSpendAmount = VerticalrActionStamina;
		break;
	case EParkourMovementType::EPM_Sprint:
		OutSpendAmount = SprintStamina;
		break;
	}
	return true;
}

void UCharacterStatComponent::UseStamina(float Amount, float DeltaTime)
{
	CurrentStamina = FMath::Clamp(CurrentStamina - Amount * DeltaTime, 0, MaxStamina);
	ShooterCharacter->GetWorldTimerManager().ClearTimer(RegenStaminaTimer);
	bCanGenerateStamina = false;
	StatOverlay->SetStaminaBarPercent(GetStaminaPercent());
}

void UCharacterStatComponent::ReGenStamina()
{
	CurrentStamina = FMath::Clamp(CurrentStamina + RegenStaminaPerSecond * UGameplayStatics::GetWorldDeltaSeconds(this), 0, MaxStamina);
	StatOverlay->SetStaminaBarPercent(GetStaminaPercent());
}

void UCharacterStatComponent::SetTrueRegenStaminaBoolean()
{
	bCanGenerateStamina = true;
}

bool UCharacterStatComponent::IsSpecificParkourAction(EParkourMovementType CurrentParkour)
{
	return CurrentParkour == EParkourMovementType::EPM_VerticalWallRun ||
		CurrentParkour == EParkourMovementType::EPM_Sprint ||
		CurrentParkour == EParkourMovementType::EPM_LedgeGrab;
}

bool UCharacterStatComponent::ApplyHealthChange(AActor* InstigatorActor, float Delta)
{
	bool bIsApplied = Super::ApplyHealthChange(InstigatorActor, Delta);
	if (bIsApplied)
	{
		StatOverlay->SetHealthBarPercent(GetHealthPercent());
	}
	return bIsApplied;
}

float UCharacterStatComponent::GetStaminaPercent()
{
	return CurrentStamina / MaxStamina;
}
