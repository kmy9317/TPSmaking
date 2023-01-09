#pragma once
UENUM(BlueprintType)
enum class EParkourMovementType : uint8
{
	EPM_None UMETA(DisplayName = "None"),
	EPM_LeftWallRun UMETA(DisplayName = "LeftWallRun"),
	EPM_RightWallRun UMETA(DisplayName = "RightWallRun"),
	EPM_VerticalWallRun UMETA(DisplayName = "VerticalWallRun"),
	EPM_LedgeGrab UMETA(DisplayName = "LedgeGrab"),
	EPM_Mantle UMETA(DisplayName = "Mantle"),
	EPM_Slide UMETA(DisplayName = "Slide"),
	EPM_Sprint UMETA(DisplayName = "Sprint"),
	EPM_Crouch UMETA(DisplayName = "Crouch"),

	EPM_MAX UMETA(DisplayName = "DefaultMax")
};