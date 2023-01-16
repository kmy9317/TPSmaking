// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "UE5Coop/Character/ShooterCharacter.h"
#include "DrawDebugHelpers.h"

UParkourMovementComponent::UParkourMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UParkourMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UParkourMovementComponent::Initialize(ACharacter* InitCharacter)
{
	Character = InitCharacter;
	if (Character == nullptr) return;

	ShooterCharacter = Cast<AShooterCharacter>(Character);
	if (ShooterCharacter == nullptr) return;

	CharacterMovement = Character->GetCharacterMovement();
	if (CharacterMovement == nullptr) return;

	DefaultGravity = CharacterMovement->GravityScale;
	DefaultGroundFriction = CharacterMovement->GroundFriction;
	DefaultBrakingDecelarationWalking = CharacterMovement->BrakingDecelerationWalking;
	DefaultWalkSpeed = CharacterMovement->MaxWalkSpeed;
	DefaultCrouchSpeed = CharacterMovement->MaxWalkSpeedCrouched;
	DefaultCapsuleHalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	DefaultUseControllerRotationYaw = Character->bUseControllerRotationYaw;
}

void UParkourMovementComponent::ParkourMovementUpdate()
{
	WallRun();
	VerticalWallRun();
	Mantle();
	Sprint();
	Slide();
}

void UParkourMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ParkourMovementUpdate();
	InterpCapsuleHalfHeight(DeltaTime);
}

bool UParkourMovementComponent::SetParkourMovementMode(EParkourMovementType NewMode)
{
	if (NewMode == CurrentParkourMovementMode) return false;
	else ParkourMovementChanged(CurrentParkourMovementMode, NewMode);

	return true;
}

void UParkourMovementComponent::MovementChanged(EMovementMode PreMovementMode, EMovementMode NewMovementMode)
{
	if (CharacterMovement)
	{
		PrevMovementMode = PreMovementMode;
		CurrentMovementMode = NewMovementMode;
		OpenGateNodes();
	}
}

void UParkourMovementComponent::ParkourMovementChanged(EParkourMovementType PrevMode, EParkourMovementType NewMode)
{
	PrevParkourMovementMode = PrevMode;
	CurrentParkourMovementMode = NewMode;
	ResetMovement();
}

void UParkourMovementComponent::InterpCapsuleHalfHeight(float DeltaTime)
{
	if (Character == nullptr) return;

	float TargetCapsuleHalfHeight{};
	if (CurrentParkourMovementMode == EParkourMovementType::EPM_Slide)
	{
		TargetCapsuleHalfHeight = SlidingCapsuleHalfHeight;
	}
	else if (CurrentParkourMovementMode == EParkourMovementType::EPM_Crouch)
	{
		TargetCapsuleHalfHeight = CrouchingCapsuleHalfHeight;
	}
	else
	{
		TargetCapsuleHalfHeight = DefaultCapsuleHalfHeight;
	}
	const float InterpHalfHeight{ FMath::FInterpTo(Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
		TargetCapsuleHalfHeight,
		DeltaTime,
		6.f) };

	// Negative value if crouching; Positive value if standing
	// Sinking if crouching, Rising if standing
	const float DeltaCapsuleHalfHeight{ InterpHalfHeight - Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() };
	const FVector MeshOffset{ 0.f, 0.f, -DeltaCapsuleHalfHeight };
	Character->GetMesh()->AddLocalOffset(MeshOffset);
	Character->GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHalfHeight);
}

void UParkourMovementComponent::ResetMovement()
{
	bool bShouldReset = CurrentParkourMovementMode == EParkourMovementType::EPM_None ||
		CurrentParkourMovementMode == EParkourMovementType::EPM_Crouch;

	if (bShouldReset)
	{
		CharacterMovement->bOrientRotationToMovement = true;
		Character->bUseControllerRotationYaw = ShooterCharacter->GetUseControlRotationYaw();
		CharacterMovement->GravityScale = DefaultGravity;
		CharacterMovement->GroundFriction = DefaultGroundFriction;
		CharacterMovement->BrakingDecelerationWalking = DefaultBrakingDecelarationWalking;
		float WalkSpeed = CurrentParkourMovementMode == EParkourMovementType::EPM_Crouch ? DefaultCrouchSpeed : DefaultWalkSpeed;
		CharacterMovement->MaxWalkSpeed = WalkSpeed;
		CharacterMovement->SetPlaneConstraintEnabled(false);
		CharacterMovement->SetMovementMode(SelectMovementMode());
	}

	// Exception case (Sprint)
	else
	{
		CharacterMovement->bOrientRotationToMovement = CurrentParkourMovementMode == EParkourMovementType::EPM_Sprint;
		Character->bUseControllerRotationYaw =
			CurrentParkourMovementMode == EParkourMovementType::EPM_Sprint &&
			DefaultUseControllerRotationYaw;
	}
}

EMovementMode UParkourMovementComponent::SelectMovementMode()
{
	switch (PrevParkourMovementMode)
	{
	case EParkourMovementType::EPM_None:
		return EMovementMode::MOVE_Walking;
	case EParkourMovementType::EPM_LeftWallRun:
		return EMovementMode::MOVE_Falling;
	case EParkourMovementType::EPM_RightWallRun:
		return EMovementMode::MOVE_Falling;
	case EParkourMovementType::EPM_VerticalWallRun:
		return EMovementMode::MOVE_Falling;
	case EParkourMovementType::EPM_LedgeGrab:
		return EMovementMode::MOVE_Falling;
	case EParkourMovementType::EPM_Mantle:;
		return EMovementMode::MOVE_Walking;
	case EParkourMovementType::EPM_Slide:
		return EMovementMode::MOVE_Walking;
	case EParkourMovementType::EPM_Sprint:
		return EMovementMode::MOVE_Walking;
	case EParkourMovementType::EPM_Crouch:
		return EMovementMode::MOVE_Walking;
	}

	return EMovementMode::MOVE_Walking;
}

float UParkourMovementComponent::ForwardInput()
{
	return FVector::DotProduct(Character->GetActorForwardVector(), CharacterMovement->GetLastInputVector());
}

void UParkourMovementComponent::JumpEvent()
{
	if (CurrentParkourMovementMode == EParkourMovementType::EPM_None)
	{
		if (!CharacterMovement->IsFalling())
		{
			OpenGates();
		}
	}
	else
	{
		WallRunJump();
		LedgeGrabJump();
		SprintJump();
		CrouchJump();
		SlideJump();
	}
}

void UParkourMovementComponent::LandEvent()
{
	EndEvents();
	CloseGates();
}

void UParkourMovementComponent::OpenGates()
{
	OpenWallRunGate();
	OpenVerticalWallRunGate();
	OpenSprintGate();
	OpenSlideGate();
}

void UParkourMovementComponent::OpenGateNodes()
{
	if (PrevMovementMode == EMovementMode::MOVE_Walking && CurrentMovementMode == EMovementMode::MOVE_Falling)
	{
		EndEvents();
		OpenGates();
	}
	else
	{

	}
}

void UParkourMovementComponent::CloseGates()
{
	CloseWallRunGate();
	CloseVerticalWallRunGate();
	CloseSprintGate();
	CloseSlideGate();
}

void UParkourMovementComponent::EndEvents()
{
	WallRunEnd(0.f);
	VerticalWallRunEnd(0.f);
	SprintEnd();
	SlideEnd();
}

void UParkourMovementComponent::CheckQueues()
{

}

bool UParkourMovementComponent::StaminaIsNotZero()
{
	if (ShooterCharacter == nullptr) return false;
	return ShooterCharacter->GetCurrentStamina() > 0.f;
}

void UParkourMovementComponent::WallRun()
{
	if (!WallRunGate.IsOpen()) return;

	WallRunUpdate();
}

void UParkourMovementComponent::OpenWallRunGate()
{
	WallRunGate.Open();
}

void UParkourMovementComponent::CloseWallRunGate()
{
	WallRunGate.Close();
}

void UParkourMovementComponent::WallRunUpdate()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	if (CanWallRun())
	{
		FVector RightEnd;
		FVector LeftEnd;
		WallRunEndVectors(RightEnd, LeftEnd);

		// Check Right
		if (WallRunMovement(Character->GetActorLocation(), RightEnd, -1.f))
		{
			if (SetParkourMovementMode(EParkourMovementType::EPM_RightWallRun))
			{
				World->GetTimerManager().SetTimer(WallRunEnableGravityTimer, this, &UParkourMovementComponent::WallRunEnableGravity, 1.f);
				CorrectWallRunPosition();
				WallRunGravity();
			}
			else WallRunGravity();
		}
		else
		{
			if (CurrentParkourMovementMode == EParkourMovementType::EPM_RightWallRun) WallRunEnd(0.5f);
			else
			{
				// Check Left
				if (WallRunMovement(Character->GetActorLocation(), LeftEnd, 1.f))
				{
					if (SetParkourMovementMode(EParkourMovementType::EPM_LeftWallRun))
					{
						World->GetTimerManager().SetTimer(WallRunEnableGravityTimer, this, &UParkourMovementComponent::WallRunEnableGravity, 1.f);
						CorrectWallRunPosition();
						WallRunGravity();
					}
					else WallRunGravity();
				}
				else WallRunEnd(0.5f);
			}
		}
	}
	else WallRunEnd(1.f);
}

void UParkourMovementComponent::CorrectWallRunPosition()
{
	if (IsWallRunning())
	{
		FVector WallRunCorrectPos =
			WallRunTargetPosition +
			WallRunNormal * Character->GetCapsuleComponent()->GetUnscaledCapsuleRadius();

		float CorrectRoll = Character->GetCapsuleComponent()->GetRelativeRotation().Roll;
		float CorrectPitch = Character->GetCapsuleComponent()->GetRelativeRotation().Pitch;
		float CorrectYaw =
			FRotationMatrix::MakeFromX(WallRunNormal).Rotator().Yaw;
		CorrectYaw = CurrentParkourMovementMode == EParkourMovementType::EPM_LeftWallRun ? CorrectYaw - 90.f : CorrectYaw + 90.f;
		FRotator WallRunCorrectRot(CorrectPitch, CorrectYaw, CorrectRoll);
		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		UKismetSystemLibrary::MoveComponentTo(
			Character->GetRootComponent(),
			WallRunCorrectPos,
			WallRunCorrectRot,
			false,
			false,
			.1f,
			true,
			EMoveComponentAction::Move,
			LatentInfo);
	}
}

bool UParkourMovementComponent::CanWallRun()
{
	return (CurrentParkourMovementMode == EParkourMovementType::EPM_None ||
		IsWallRunning() ||
		ShooterCharacter->GetAO_Yaw() < 89.f ||
		ShooterCharacter->GetAO_Yaw() > -89.f) && ForwardInput() > 0.f;
}

bool UParkourMovementComponent::IsWallRunning()
{
	return CurrentParkourMovementMode == EParkourMovementType::EPM_LeftWallRun ||
		CurrentParkourMovementMode == EParkourMovementType::EPM_RightWallRun;
}

void UParkourMovementComponent::WallRunEndVectors(FVector& OutRight, FVector& OutLeft)
{
	OutRight = Character->GetActorLocation() +
		(Character->GetActorRightVector() * 75.f) +
		(Character->GetActorForwardVector() * -35.f);

	OutLeft = Character->GetActorLocation() +
		(Character->GetActorRightVector() * -75.f) +
		(Character->GetActorForwardVector() * -35.f);
}

bool UParkourMovementComponent::WallRunMovement(const FVector& Start, const FVector& End, float WallRunDirection)
{
	UWorld* World = GetWorld();
	if (World == nullptr) return false;

	FHitResult HitResult;
	World->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility);
	if (HitResult.bBlockingHit)
	{
		WallRunNormal = HitResult.Normal;
		WallRunTargetPosition = HitResult.ImpactPoint;

		bool bShouldLaunchCharacter = IsValidWallVector(WallRunNormal) &&
			CharacterMovement->IsFalling();

		if (bShouldLaunchCharacter)
		{
			//bool bZOverride = !bWallRunGravity && !IsWallRunning();
			FVector LaunchDirection = FVector::CrossProduct(WallRunNormal, FVector(0.f, 0.f, 1.f));
			float SpeedScale = bSprintQueued ? WallRunSprintSpeed : WallRunSpeed;
			Character->LaunchCharacter(LaunchDirection * (SpeedScale * WallRunDirection), true, true);

			return true;
		}
		else return false;
	}
	else return false;
}

bool UParkourMovementComponent::IsValidWallVector(const FVector& HitNormal)
{
	return UKismetMathLibrary::InRange_FloatFloat(HitNormal.Z, -0.3f, 0.3f);
}

void UParkourMovementComponent::WallRunEnableGravity()
{
	bWallRunGravity = IsWallRunning();
}

void UParkourMovementComponent::WallRunGravity()
{
	CharacterMovement->GravityScale =
		FMath::FInterpTo(
			CharacterMovement->GravityScale,
			WallRunTargetGravity,
			UGameplayStatics::GetWorldDeltaSeconds(this),
			10.f
		);
}

void UParkourMovementComponent::WallRunJump()
{
	if (IsWallRunning())
	{
		WallRunEnd(0.35f);
		FVector LaunchVelocity(WallRunJumpOffForce * WallRunNormal.X, WallRunJumpOffForce * WallRunNormal.Y, WallRunJumpHeight);
		Character->LaunchCharacter(LaunchVelocity, false, true);
	}
}

void UParkourMovementComponent::WallRunEnd(float ResetTime)
{
	UWorld* World = GetWorld();

	if (IsWallRunning())
	{
		bool bChange = SetParkourMovementMode(EParkourMovementType::EPM_None);
		if (bChange && World)
		{
			WallRunGate.Close();
			World->GetTimerManager().SetTimer(OpenWallRunTimer, this, &UParkourMovementComponent::OpenWallRunGate, ResetTime);
			World->GetTimerManager().ClearTimer(WallRunEnableGravityTimer);
			bWallRunGravity = false;
		}
	}
}

void UParkourMovementComponent::VerticalWallRun()
{
	if (!VerticalWallRunGate.IsOpen()) return;

	VerticalWallRunUpdate();
}

void UParkourMovementComponent::OpenVerticalWallRunGate()
{
	VerticalWallRunGate.Open();
}

void UParkourMovementComponent::CloseVerticalWallRunGate()
{
	VerticalWallRunGate.Close();
	CloseMantleGate();
}

void UParkourMovementComponent::VerticalWallRunUpdate()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	if (CanVerticalWallRun())
	{
		FVector Eyes;
		FVector Feet;
		MantleVectors(Eyes, Feet);
		TArray<AActor*> ActorsToIgnore;
		FHitResult HitResult;
		UKismetSystemLibrary::CapsuleTraceSingle(
			this,
			Eyes,
			Feet,
			20.f,
			10.f,
			ETraceTypeQuery::TraceTypeQuery1,
			false,
			ActorsToIgnore,
			EDrawDebugTrace::None,
			HitResult,
			true
		);

		if (HitResult.bBlockingHit)
		{
			MantleTraceDistance = HitResult.Distance;
			LedgeFloorTargetPosition = HitResult.ImpactPoint;
			FHitResult ForwardHit;
			ForwardTracer(ForwardHit);
			if (CharacterMovement->IsWalkable(HitResult) && ForwardHit.bBlockingHit)
			{
				LedgeClimbWallTargetPosition = ForwardHit.ImpactPoint;
				LedgeClimbWallTargetNormal = ForwardHit.ImpactNormal;
				MantlePosition = LedgeFloorTargetPosition +
					FVector(0.f, 0.f, Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
				CloseVerticalWallRunGate();
				LedgeGrab();
				bLedgeCloseToGround = IsLedgeCloseToGround();
				if (CanQuickMantle())
				{
					OpenMantleCheckGate();
				}
				else
				{
					CorrectLedgeClimbPosition();
					World->GetTimerManager().SetTimer(
						OpenMantleCheckTimer,
						this,
						&UParkourMovementComponent::OpenMantleCheckGate,
						.25f
					);
				}
			}
			else
			{
				VerticalWallRunMovement();
			}
		}
		else VerticalWallRunMovement();
	}
	else VerticalWallRunEnd(0.35f);
}

bool UParkourMovementComponent::CanVerticalWallRun()
{
	return (CurrentParkourMovementMode == EParkourMovementType::EPM_None ||
		CurrentParkourMovementMode == EParkourMovementType::EPM_VerticalWallRun ||
		IsWallRunning()) &&
		ForwardInput() > 0.f &&
		StaminaIsNotZero() &&
		CharacterMovement->IsFalling();
}

void UParkourMovementComponent::ForwardTracer(FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FVector Eyes;
		FVector Feet;
		MantleVectors(Eyes, Feet);
		FVector Forward = Character->GetActorForwardVector() * 50.f;
		FVector End = Feet + Forward;
		FCollisionQueryParams Query;
		Query.AddIgnoredActor(Character);

		World->LineTraceSingleByChannel(
			OutHit,
			Feet,
			End,
			ECollisionChannel::ECC_Visibility,
			Query
		);
	}
}

void UParkourMovementComponent::VerticalWallRunMovement()
{
	FHitResult HitResult;
	ForwardTracer(HitResult);

	if (ForwardInput() > 0.f && HitResult.bBlockingHit)
	{
		VerticalWallRunTargetPosition = HitResult.ImpactPoint;
		VerticalWallRunNormal = HitResult.ImpactNormal;
		if (SetParkourMovementMode(EParkourMovementType::EPM_VerticalWallRun))
		{
			CorrectVerticalWallRunPosition();
		}
		FVector LaunchVelocity = VerticalWallRunNormal * -600.f;
		LaunchVelocity.Z = VerticalWallRunSpeed;
		Character->LaunchCharacter(LaunchVelocity, true, true);
	}
	else VerticalWallRunEnd(.35f);

}

void UParkourMovementComponent::CorrectVerticalWallRunPosition()
{
	if (CurrentParkourMovementMode == EParkourMovementType::EPM_VerticalWallRun)
	{
		FVector VerticalWallRunCorrectPos =
			VerticalWallRunTargetPosition +
			VerticalWallRunNormal * Character->GetCapsuleComponent()->GetUnscaledCapsuleRadius();

		float CorrectRoll = Character->GetCapsuleComponent()->GetRelativeRotation().Roll;
		float CorrectPitch = Character->GetCapsuleComponent()->GetRelativeRotation().Pitch;
		float CorrectYaw =
			FRotationMatrix::MakeFromX(VerticalWallRunNormal).Rotator().Yaw;
		CorrectYaw = CorrectYaw - 180.f;
		FRotator VerticalWallRunCorrectRot(CorrectPitch, CorrectYaw, CorrectRoll);
		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		UKismetSystemLibrary::MoveComponentTo(
			Character->GetRootComponent(),
			VerticalWallRunCorrectPos,
			VerticalWallRunCorrectRot,
			false,
			false,
			.1f,
			true,
			EMoveComponentAction::Move,
			LatentInfo);
	}
}

void UParkourMovementComponent::LedgeGrab()
{
	if (SetParkourMovementMode(EParkourMovementType::EPM_LedgeGrab))
	{
		CharacterMovement->DisableMovement();
		CharacterMovement->StopMovementImmediately();
		CharacterMovement->GravityScale = 0.f;
	}
}

bool UParkourMovementComponent::IsLedgeCloseToGround()
{
	UWorld* World = GetWorld();
	if (World)
	{
		FHitResult HitResult;
		FVector Start = Character->GetActorLocation();
		FVector End = Start -
			(Character->GetActorUpVector() * (Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 40.f));
		FCollisionQueryParams Query;
		Query.AddIgnoredActor(Character);

		World->LineTraceSingleByChannel(
			HitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility,
			Query
		);

		return HitResult.bBlockingHit;
	}

	return false;
}

void UParkourMovementComponent::CorrectLedgeClimbPosition()
{
	if (CurrentParkourMovementMode == EParkourMovementType::EPM_LedgeGrab)
	{
		FVector LedgeClimbCorrectPos =
			LedgeClimbWallTargetPosition +
			LedgeClimbWallTargetNormal * Character->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
		LedgeClimbCorrectPos.Z = LedgeFloorTargetPosition.Z - Character->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();

		float CorrectRoll = Character->GetCapsuleComponent()->GetRelativeRotation().Roll;
		float CorrectPitch = Character->GetCapsuleComponent()->GetRelativeRotation().Pitch;
		float CorrectYaw =
			FRotationMatrix::MakeFromX(LedgeClimbWallTargetNormal).Rotator().Yaw;
		CorrectYaw = CorrectYaw - 180.f;
		FRotator LedgeClimbCorrectRot(CorrectPitch, CorrectYaw, CorrectRoll);
		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		UKismetSystemLibrary::MoveComponentTo(
			Character->GetRootComponent(),
			LedgeClimbCorrectPos,
			LedgeClimbCorrectRot,
			false,
			false,
			.1f,
			true,
			EMoveComponentAction::Move,
			LatentInfo);
	}
}

void UParkourMovementComponent::LedgeGrabJump()
{
	bool bCanLedgeGrabJump =
		CurrentParkourMovementMode == EParkourMovementType::EPM_LedgeGrab ||
		CurrentParkourMovementMode == EParkourMovementType::EPM_VerticalWallRun ||
		CurrentParkourMovementMode == EParkourMovementType::EPM_Mantle;
	if (bCanLedgeGrabJump)
	{
		VerticalWallRunEnd(.36f);
		FVector LaunchVelocity(
			VerticalWallRunNormal.X * LedgeGrabJumpOffForce,
			VerticalWallRunNormal.Y * LedgeGrabJumpOffForce,
			LedgeGrabJumpHeight);
		Character->LaunchCharacter(LaunchVelocity, false, true);
	}
}

void UParkourMovementComponent::VerticalWallRunEnd(float ResetTime)
{
	UWorld* World = GetWorld();

	bool bCheckVerticalWallRunEnd =
		CurrentParkourMovementMode == EParkourMovementType::EPM_VerticalWallRun ||
		CurrentParkourMovementMode == EParkourMovementType::EPM_LedgeGrab ||
		CurrentParkourMovementMode == EParkourMovementType::EPM_Mantle;

	if (bCheckVerticalWallRunEnd && World)
	{
		if (SetParkourMovementMode(EParkourMovementType::EPM_None))
		{
			CloseVerticalWallRunGate();
			CloseMantleCheckGate();
			World->GetTimerManager().SetTimer(OpenVerticalWallRunTimer, this, &UParkourMovementComponent::OpenVerticalWallRunGate, ResetTime);
			World->GetTimerManager().SetTimer(VerticalWallRunEndCheckQueuesTimer, this, &UParkourMovementComponent::CheckQueues, .2f);
		}
	}
}

void UParkourMovementComponent::Mantle()
{
	if (MantleCheckGate.IsOpen())
	{
		MantleCheck();
	}
	if (MantleGate.IsOpen())
	{
		MantleMovement();
	}
}

void UParkourMovementComponent::OpenMantleCheckGate()
{
	MantleCheckGate.Open();
}

void UParkourMovementComponent::CloseMantleCheckGate()
{
	MantleCheckGate.Close();
}

void UParkourMovementComponent::OpenMantleGate()
{
	MantleGate.Open();
}

void UParkourMovementComponent::CloseMantleGate()
{
	MantleGate.Close();
}

void UParkourMovementComponent::MantleVectors(FVector& OutEyes, FVector& OutFeet)
{
	if (Character->GetController() && Character->GetCapsuleComponent())
	{
		FVector EyesPos;
		FRotator EyesRot;
		Character->GetController()->GetActorEyesViewPoint(EyesPos, EyesRot);
		EyesPos.Z += 50.f;
		FVector Forward = Character->GetActorForwardVector() * Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
		FVector FeetPos(Character->GetActorLocation().X, Character->GetActorLocation().Y,
			Character->GetActorLocation().Z - Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + MantleHeight);


		OutEyes = EyesPos + Forward;
		OutFeet = FeetPos + Forward;
	}
}

bool UParkourMovementComponent::CanQuickMantle()
{
	return MantleTraceDistance > Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() ||
		bLedgeCloseToGround;
}

bool UParkourMovementComponent::CanMantle()
{
	return (CurrentParkourMovementMode == EParkourMovementType::EPM_LedgeGrab || CanQuickMantle()) &&
		ForwardInput() > 0.f;
}

void UParkourMovementComponent::MantleCheck()
{
	if (CanMantle())
	{
		MantleStart();
	}
	else if (CurrentParkourMovementMode == EParkourMovementType::EPM_LedgeGrab && !StaminaIsNotZero())
	{
		VerticalWallRunEnd(0.36f);
	}
}

void UParkourMovementComponent::MantleStart()
{
	if (SetParkourMovementMode(EParkourMovementType::EPM_Mantle))
	{
		CloseMantleCheckGate();
		OpenMantleGate();
	}
}

void UParkourMovementComponent::MantleMovement()
{
	if (Character->GetController())
	{
		FRotator CurrentRot = Character->GetController()->GetControlRotation();
		FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(
			FVector(Character->GetActorLocation().X, Character->GetActorLocation().Y, 0.f),
			FVector(MantlePosition.X, MantlePosition.Y, 0.f)
		);

		FRotator InterpRot = FMath::RInterpTo(CurrentRot, TargetRot, UGameplayStatics::GetWorldDeltaSeconds(this), 7.f);

		Character->GetController()->SetControlRotation(InterpRot);

		FVector CurrentPos = Character->GetActorLocation();
		FVector TargetPos = MantlePosition;
		float InterpSpeed = CanQuickMantle() ? QuickMantleSpeed : MantleSpeed;
		FVector InterpPos = FMath::VInterpTo(CurrentPos, TargetPos, UGameplayStatics::GetWorldDeltaSeconds(this), InterpSpeed);
		Character->SetActorLocation(InterpPos);

		if (FVector::Distance(Character->GetActorLocation(), MantlePosition) < 8.f) VerticalWallRunEnd(.5f);
	}
}

void UParkourMovementComponent::Sprint()
{
	if (SprintGate.IsOpen())
	{
		SprintUpdate();
	}
}

void UParkourMovementComponent::SprintEvent()
{
	SprintStart();
}

void UParkourMovementComponent::SprintStart()
{
	if (CanSprint())
	{
		if (SetParkourMovementMode(EParkourMovementType::EPM_Sprint))
		{
			CharacterMovement->MaxWalkSpeed = SprintSpeed;
			OpenSprintGate();
		}
	}
}

void UParkourMovementComponent::OpenSprintGate()
{
	SprintGate.Open();
}

void UParkourMovementComponent::CloseSprintGate()
{
	SprintGate.Close();
}

void UParkourMovementComponent::SprintUpdate()
{
	if (CurrentParkourMovementMode == EParkourMovementType::EPM_Sprint && !(ForwardInput() > 0.f) || !StaminaIsNotZero())
	{
		SprintEnd();
	}
}

bool UParkourMovementComponent::CanSprint()
{
	return CurrentParkourMovementMode == EParkourMovementType::EPM_None && CharacterMovement->IsWalking();
}

void UParkourMovementComponent::SprintJump()
{
	if (CurrentParkourMovementMode == EParkourMovementType::EPM_Sprint)
	{
		SprintEnd();
	}
}

void UParkourMovementComponent::SprintEnd()
{
	UWorld* World = GetWorld();

	if (CurrentParkourMovementMode == EParkourMovementType::EPM_Sprint && World)
	{
		if (SetParkourMovementMode(EParkourMovementType::EPM_None))
		{
			CloseSprintGate();
			World->GetTimerManager().SetTimer(OpenSprintTimer, this, &UParkourMovementComponent::OpenSprintGate, .1f);
		}
	}
}

void UParkourMovementComponent::CrouchSlideEvent()
{
	ParkourCancel();
	if (CanSlide() && CharacterMovement->IsWalking())
	{
		SlideStart();
	}
	else
	{
		CrouchToggle();
	}
}

void UParkourMovementComponent::ParkourCancel()
{
	bool bVerticalParkour = CurrentParkourMovementMode == EParkourMovementType::EPM_Mantle ||
		CurrentParkourMovementMode == EParkourMovementType::EPM_VerticalWallRun ||
		CurrentParkourMovementMode == EParkourMovementType::EPM_LedgeGrab;

	if (bVerticalParkour)
	{
		VerticalWallRunEnd(.5f);
	}
	else if (IsWallRunning())
	{
		WallRunEnd(.5f);
	}
}

void UParkourMovementComponent::CrouchToggle()
{
	switch (CurrentParkourMovementMode)
	{
	case EParkourMovementType::EPM_None:
		CrouchStart();
		break;
	case EParkourMovementType::EPM_Crouch:
		CrouchEnd();
		break;
	}
}

void UParkourMovementComponent::CrouchStart()
{
	if (CurrentParkourMovementMode == EParkourMovementType::EPM_None)
	{
		SetParkourMovementMode(EParkourMovementType::EPM_Crouch);
		CharacterMovement->MaxWalkSpeed = DefaultCrouchSpeed;
	}
}

void UParkourMovementComponent::CrouchJump()
{
	if (CurrentParkourMovementMode == EParkourMovementType::EPM_Crouch)
	{
		CrouchEnd();
	}
}

void UParkourMovementComponent::CrouchEnd()
{
	if (CurrentParkourMovementMode == EParkourMovementType::EPM_Crouch)
	{
		SetParkourMovementMode(EParkourMovementType::EPM_None);
		CharacterMovement->MaxWalkSpeed = DefaultWalkSpeed;
	}
}

void UParkourMovementComponent::Slide()
{
	if (SlideGate.IsOpen())
	{
		SlideUpdate();
	}
}

void UParkourMovementComponent::SlideUpdate()
{
	if (ShouldSlideEnd())
	{
		SlideEnd(true);
	}
}

void UParkourMovementComponent::SlideStart()
{
	if (CanSlide() && CharacterMovement->IsWalking())
	{
		SprintEnd();
		SetParkourMovementMode(EParkourMovementType::EPM_Slide);
		CharacterMovement->GroundFriction = 0.f;
		CharacterMovement->BrakingDecelerationWalking = SlidingBrakingDeceleration;
		CharacterMovement->MaxWalkSpeed = 0.f;
		CharacterMovement->SetPlaneConstraintFromVectors(CharacterMovement->Velocity.GetSafeNormal(),
			Character->GetActorUpVector());
		CharacterMovement->SetPlaneConstraintEnabled(true);
		FVector SlideVector = GetSlideVector();

		if (SlideVector.Z <= 0.2f)
		{
			CharacterMovement->AddImpulse(SlideVector * SlidingImpulseAmount, true);
		}
		OpenSlideGate();
	}
}

void UParkourMovementComponent::OpenSlideGate()
{
	SlideGate.Open();
}

void UParkourMovementComponent::CloseSlideGate()
{
	SlideGate.Close();
}

bool UParkourMovementComponent::CanSlide()
{
	return CurrentParkourMovementMode == EParkourMovementType::EPM_Sprint &&
		ForwardInput() > 0.f;
}

FVector UParkourMovementComponent::GetSlideVector()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return FVector();

	FVector Start = Character->GetActorLocation();
	FVector End = Character->GetActorLocation() + Character->GetActorUpVector() * -200.f;
	FHitResult HitResult;
	FCollisionQueryParams Query;
	Query.AddIgnoredActor(Character);
	World->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, Query);

	if (HitResult.bBlockingHit)
	{
		return FVector::CrossProduct(HitResult.ImpactNormal, Character->GetActorRightVector()) * -1.f;
	}

	return FVector();
}

void UParkourMovementComponent::SlideJump()
{
	if (CurrentParkourMovementMode == EParkourMovementType::EPM_Slide)
	{
		SlideEnd();
	}
}

bool UParkourMovementComponent::ShouldSlideEnd()
{
	return (CurrentParkourMovementMode == EParkourMovementType::EPM_Slide && CharacterMovement->Velocity.Length() <= 35.f) ||
		ShooterCharacter->GetAO_Yaw() > 89.f ||
		ShooterCharacter->GetAO_Yaw() < -89.f;
}

void UParkourMovementComponent::SlideEnd(bool bCrouch)
{
	if (CurrentParkourMovementMode == EParkourMovementType::EPM_Slide)
	{
		EParkourMovementType NewMode = bCrouch ? EParkourMovementType::EPM_Crouch : EParkourMovementType::EPM_None;
		if (SetParkourMovementMode(NewMode))
		{
			CloseSlideGate();
		}
	}
}

