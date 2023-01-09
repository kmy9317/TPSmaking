// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Owned UMETA(DisplayName "Owned"),
	EWS_Dropped UMETA(DisplayName "Dropped")
};

UCLASS()
class UE5COOP_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void Fire(const FVector& HitTarget);

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsBottom;

	UPROPERTY(EditAnywhere, Category = "Fire Delay")
	float FireDelay = 1.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	bool bAutomatic = true;

protected:
	virtual void BeginPlay() override;

	void SetMaterialInstanceDynamics();
	TArray<UMaterialInstanceDynamic*> MaterialInstanceDynamics;

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Projectiles")
	USkeletalMeshComponent* WeaponMesh;

public:	
	void SetWeaponState(EWeaponState WeaponState);
	void SetMaterialParamters(float Opacity);
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
};
