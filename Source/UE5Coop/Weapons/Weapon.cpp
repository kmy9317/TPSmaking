// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Particles/ParticleSystemComponent.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	SetMaterialInstanceDynamics();
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::Fire(const FVector& HitTarget)
{

}

void AWeapon::SetMaterialInstanceDynamics()
{
	TArray<UMaterialInterface*> Materials = WeaponMesh->GetMaterials();

	if (Materials.IsEmpty()) return;

	for (int32 Index = 0; Index < Materials.Num(); Index++)
	{
		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Materials[Index], this);
		if (DynamicMaterial)
		{
			UE_LOG(LogTemp, Warning, TEXT("DynamicMaterial"));
			MaterialInstanceDynamics.Add(DynamicMaterial);
			WeaponMesh->SetMaterial(Index, DynamicMaterial);
		}
	}
}

void AWeapon::SetMaterialParamters(float Opacity)
{
	if (MaterialInstanceDynamics.IsEmpty()) return;

	for (auto Material : MaterialInstanceDynamics)
	{
		Material->SetScalarParameterValue(TEXT("Opacity"), Opacity);
	}
}

void AWeapon::SetWeaponState(EWeaponState WeaponState)
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Owned:
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	}
}




