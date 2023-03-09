// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UE5Coop/ParkourMovementMode//ParkourMovementMode.h"
#include "StatComponent.generated.h"

class AShooterCharacter;
class UStatOverlay;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHealthChanged, AActor*, InstigatorActor, UStatComponent*, OwningComp, float, NewHealth, float, Delta);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UE5COOP_API UStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	friend class AShooterCharacter;

	UPROPERTY(BlueprintAssignable)
	FOnHealthChanged OnHealthChanged;

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable)
	virtual bool ApplyHealthChange(AActor* InstigatorActor, float Delta);

private:

	// Health
	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float MaxHp = 100.f;

	UPROPERTY(VisibleAnywhere, Category = "Health")
	float CurrentHp = 100.f;

public:
	float GetHealthPercent();
};