#include "StatComponent.h"
#include "Kismet/GameplayStatics.h"

void UStatComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UStatComponent::ApplyHealthChange(AActor* InstigatorActor, float Delta)
{
	 float PrevHp = CurrentHp;
	 CurrentHp = FMath::Clamp(CurrentHp + Delta, 0.f, MaxHp);

	 float ActualDelta = CurrentHp - PrevHp;

	 OnHealthChanged.Broadcast(InstigatorActor, this, CurrentHp, ActualDelta);

	 return true;
}

float UStatComponent::GetHealthPercent()
{
	return CurrentHp / MaxHp;
}
