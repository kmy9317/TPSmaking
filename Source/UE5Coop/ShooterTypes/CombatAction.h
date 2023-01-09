#pragma once

UENUM(BlueprintType)
enum class ECombatAction : uint8
{
	ECA_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECA_Swapping UMETA(DisplayName = "Swapping"),

	ECA_MAX UMETA(DisplayName = "DefaultMax")
};