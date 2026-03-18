// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusApiNotifyAction.h
 * @brief Enum controlling how REST API operation results are reported to the user.
 */

#pragma once

#include "CoreMinimal.h"

#include "CavrnusApiNotifyAction.generated.h"		// Always last

/**
 * @brief Controls what happens with success/error messages from API operations.
 */
UENUM(BlueprintType, Category = "Cavrnus|API")
enum class ECavrnusApiNotifyAction : uint8
{
	/** Do nothing — caller handles the result via callbacks only. */
	None		UMETA(DisplayName = "None"),

	/** Write the result to the output log (UE_LOG). */
	LogOnly		UMETA(DisplayName = "Log Only"),

	/** Show an auto-dismissing toast message in the viewport. */
	Toast		UMETA(DisplayName = "Toast")
};
