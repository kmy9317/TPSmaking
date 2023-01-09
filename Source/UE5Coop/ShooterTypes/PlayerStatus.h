#pragma once

UENUM(BlueprintType)
enum class EPlayerStatus : uint8
{
	EPS_Walking UMETA(DisplayName = "Walking"),
	EPS_Sprint UMETA(DisplayName = "Sprint"),
	EPS_Sliding UMETA(DisplayName = "Sliding"),
	EPS_Crouching UMETA(DisplayName = "Crouching"),
	EPS_Flying UMETA(DisplayName = "Flying"),

	EPS_MAX UMETA(DisplayName = "DefaultMax")
};