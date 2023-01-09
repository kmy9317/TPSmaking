#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Combat UMETA(DisplayName "Combat"),
	ECS_Travel UMETA(DisplayName "Travel"),
	ECS_NonCombat UMETA(DisplayName "NonCombat"),
	

	ECS_MAX UMETA(DisplayName "DefaultMAX")

};