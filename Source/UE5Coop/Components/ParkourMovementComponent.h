// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UE5Coop/FlowControlNode/Gate.h"
#include "UE5Coop/ParkourMovementMode/ParkourMovementMode.h"
#include "ParkourMovementComponent.generated.h"

class ACharacter;
class UCharacterMovementComponent;
class AShooterCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UE5COOP_API UParkourMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	friend class AShooterCharacter;

	UParkourMovementComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	void Initialize(ACharacter* Character);
	void ParkourMovementUpdate();

	// Common
	bool SetParkourMovementMode(EParkourMovementType NewMode);
	void MovementChanged(EMovementMode PreMovementMode, EMovementMode NewMovementMode);
	void ParkourMovementChanged(EParkourMovementType PrevMode, EParkourMovementType NewMode);
	void InterpCapsuleHalfHeight(float DeltaTime);
	void ResetMovement();
	EMovementMode SelectMovementMode();
	float ForwardInput();
	void JumpEvent();
	void LandEvent();
	void OpenGates();
	void OpenGateNodes();
	void CloseGates();
	void EndEvents();
	void CheckQueues();

	// Wall Run
	void WallRun();
	void OpenWallRunGate();
	void CloseWallRunGate();
	void WallRunUpdate();
	void CorrectWallRunPosition();
	bool CanWallRun();
	bool IsWallRunning();
	void WallRunEndVectors(FVector& OutRight, FVector& OutLeft);
	bool WallRunMovement(const FVector& Start, const FVector& End, float WallRunDirection);
	bool IsValidWallVector(const FVector& HitNormal);
	void WallRunEnableGravity();
	void WallRunGravity();
	void WallRunJump();
	void WallRunEnd(float ResetTime);

	// Vertical Wall Run && Ledge
	void VerticalWallRun();
	void OpenVerticalWallRunGate();
	void CloseVerticalWallRunGate();
	void VerticalWallRunUpdate();
	bool CanVerticalWallRun();
	void VerticalWallRunMovement();
	void ForwardTracer(FHitResult& OutHit);
	void CorrectVerticalWallRunPosition();
	void LedgeGrab();
	bool IsLedgeCloseToGround();
	void CorrectLedgeClimbPosition();
	void LedgeGrabJump();
	void VerticalWallRunEnd(float ResetTime);

	// Mantle
	void Mantle();
	void OpenMantleCheckGate();
	void CloseMantleCheckGate();
	void OpenMantleGate();
	void CloseMantleGate();
	void MantleVectors(FVector& OutEyes, FVector& OutFeet);
	bool CanQuickMantle();
	bool CanMantle();
	void MantleCheck();
	void MantleStart();
	void MantleMovement();

	// Sprint
	void Sprint();
	void SprintEvent();
	void SprintStart();
	void OpenSprintGate();
	void CloseSprintGate();
	void SprintUpdate();
	bool CanSprint();
	void SprintJump();
	void SprintEnd();

	// Crouch && Slide
	void CrouchSlideEvent();
	void ParkourCancel();
	void CrouchToggle();
	void CrouchStart();
	void CrouchJump();
	void CrouchEnd();	
	void Slide();
	void SlideUpdate();
	void SlideStart();
	void OpenSlideGate();
	void CloseSlideGate();
	bool CanSlide();
	FVector GetSlideVector();
	void SlideJump();
	void SlideEnd(bool bCrouch = false);

private:

	UPROPERTY(VisibleAnywhere, Category = Character)
	ACharacter* Character;

	UPROPERTY(VisibleAnywhere, Category = Character)
	AShooterCharacter* ShooterCharacter;

	UPROPERTY(VisibleAnywhere, Category = Character)
	UCharacterMovementComponent* CharacterMovement;

	EMovementMode PrevMovementMode;
	EMovementMode CurrentMovementMode;

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	EParkourMovementType PrevParkourMovementMode;

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	EParkourMovementType CurrentParkourMovementMode;

	// Gates
	UPROPERTY(VisibleAnywhere)
	FGate WallRunGate = FGate(true);

	UPROPERTY(VisibleAnywhere)
	FGate VerticalWallRunGate = FGate(true);

	UPROPERTY(VisibleAnywhere)
	FGate MantleCheckGate = FGate(true);

	UPROPERTY(VisibleAnywhere)
	FGate MantleGate = FGate(true);

	UPROPERTY(VisibleAnywhere)
	FGate SprintGate = FGate(true);

	UPROPERTY(VisibleAnywhere)
	FGate SlideGate = FGate(true);

	// Default Values
	float DefaultGravity;
	float DefaultGroundFriction;
	float DefaultBrakingDecelarationWalking;
	float DefaultWalkSpeed;
	float DefaultCrouchSpeed;
	bool DefaultUseControllerRotationYaw;

	FTimerHandle UpdateTimer;
	UPROPERTY(EditDefaultsOnly)
	float UpdateDelay = 0.0167f;
	
	// Wall Run Variables
	FVector WallRunNormal;
	FVector WallRunTargetPosition;
	bool bWallRunGravity = false;
	FTimerHandle OpenWallRunTimer;
	FTimerHandle WallRunEnableGravityTimer;

	UPROPERTY(EditDefaultsOnly, Category = "WallRun")
	float WallRunSpeed = 850.f;

	UPROPERTY(EditDefaultsOnly, Category = "WallRun")
	float WallRunSprintSpeed = 1100.f;

	UPROPERTY(EditDefaultsOnly, Category = "WallRun")
	float WallRunTargetGravity = 0.25f;

	UPROPERTY(EditDefaultsOnly, Category = "WallRun")
	float WallRunJumpOffForce = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "WallRun")
	float WallRunJumpHeight = 400.f;

	// Vertial Wall Run && Ledge Variables
	bool bLedgeCloseToGround = false;
	FTimerHandle OpenVerticalWallRunTimer;
	FTimerHandle VerticalWallRunEndCheckQueuesTimer;
	FVector VerticalWallRunTargetPosition;
	FVector VerticalWallRunNormal;
	FVector LedgeFloorTargetPosition;
	FVector LedgeClimbWallTargetPosition;
	FVector LedgeClimbWallTargetNormal;

	UPROPERTY(EditDefaultsOnly, Category = "VerticalWallRun")
	float VerticalWallRunSpeed = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "LedgeGrab")
	float LedgeGrabJumpOffForce = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "LedgeGrab")
	float LedgeGrabJumpHeight = 400.f;

	// Mantle Variables
	float MantleTraceDistance;
	FVector MantlePosition;
	FTimerHandle OpenMantleCheckTimer;

	UPROPERTY(EditDefaultsOnly, Category = "Mantle")
	float MantleHeight = 40.f;

	UPROPERTY(EditDefaultsOnly, Category = "Mantle")
	float MantleSpeed = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "Mantle")
	float QuickMantleSpeed = 20.f;

	// Sprint Variables
	bool bSprintQueued = false;
	FTimerHandle OpenSprintTimer;

	UPROPERTY(EditDefaultsOnly, Category = "Sprint")
	float SprintSpeed = 1000.f;

	// Crouch && Slide Variables
	float DefaultCapsuleHalfHeight;

	UPROPERTY(EditDefaultsOnly, Category = "CrouchAndSlide")
	float CrouchingCapsuleHalfHeight = 60.f;

	UPROPERTY(EditDefaultsOnly, Category = "CrouchAndSlide")
	float SlidingCapsuleHalfHeight = 60.f;

	UPROPERTY(EditDefaultsOnly, Category = "CrouchAndSlide")
	float SlidingBrakingDeceleration = 1400.f;

	UPROPERTY(EditDefaultsOnly, Category = "CrouchAndSlide")
	float SlidingImpulseAmount = 600.f;
};
