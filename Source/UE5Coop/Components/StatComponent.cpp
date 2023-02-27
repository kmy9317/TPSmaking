#include "StatComponent.h"
#include "UE5Coop/Character/ShooterCharacter.h"
#include "UE5Coop/HUD/StatOverlay.h"
#include "UE5Coop/HUD/ShooterHUD.h"
#include "Kismet/GameplayStatics.h"

UStatComponent::UStatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ShooterCharacter)
	{
		InitStatOverlay();
	}
}

void UStatComponent::InitStatOverlay()
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

void UStatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CheckStamina(DeltaTime);
}

bool UStatComponent::ApplyHealthChange(float Delta)
{
	 float CalculatedHP = FMath::Clamp(CurrentHp + Delta, 0.f, MaxHp);

	 if (CalculatedHP == CurrentHp) return false;
	 
	 CurrentHp = CalculatedHP;
	 StatOverlay->SetHealthBarPercent(GetHealthPercent());
	 return true;
}

void UStatComponent::CheckStamina(float DeltaTime)
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
			&UStatComponent::SetTrueRegenStaminaBoolean,
			RegenStaminaTime);
	}
	else if (bCanGenerateStamina)
	{
		ReGenStamina();
	}
}

bool UStatComponent::ShouldUseStamina(float& OutSpendAmount)
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

void UStatComponent::UseStamina(float Amount, float DeltaTime)
{
	CurrentStamina = FMath::Clamp(CurrentStamina - Amount * DeltaTime, 0, MaxStamina);
	ShooterCharacter->GetWorldTimerManager().ClearTimer(RegenStaminaTimer);
	bCanGenerateStamina = false;
	StatOverlay->SetStaminaBarPercent(GetStaminaPercent());
}

void UStatComponent::ReGenStamina()
{
	CurrentStamina = FMath::Clamp(CurrentStamina + RegenStaminaPerSecond * UGameplayStatics::GetWorldDeltaSeconds(this), 0, MaxStamina);
	StatOverlay->SetStaminaBarPercent(GetStaminaPercent());
}

void UStatComponent::SetTrueRegenStaminaBoolean()
{
	bCanGenerateStamina = true;
}

bool UStatComponent::IsSpecificParkourAction(EParkourMovementType CurrentParkour)
{
	return CurrentParkour == EParkourMovementType::EPM_VerticalWallRun ||
		CurrentParkour == EParkourMovementType::EPM_Sprint ||
		CurrentParkour == EParkourMovementType::EPM_LedgeGrab;
}

float UStatComponent::GetHealthPercent()
{
	return CurrentHp / MaxHp;
}

float UStatComponent::GetStaminaPercent()
{
	return CurrentStamina / MaxStamina;
}
