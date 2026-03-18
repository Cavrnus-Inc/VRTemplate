// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusSpaceInfoChangeFlags.h
 * @brief This file defines the ESpaceInfoChangeFlags enum used for tracking which fields of a space have changed.
 */

#pragma once

#include "CoreMinimal.h"
#include "CavrnusSpaceInfoChangeFlags.generated.h"

/**
 * @brief Bitmask flags indicating which fields of a space info have changed.
 *
 * Use these flags with BindSpaceInfoChanged to subscribe to specific types of changes.
 * Multiple flags can be combined using bitwise OR.
 */
UENUM(BlueprintType, Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ESpaceInfoChangeFlags : uint8
{
	None        = 0         UMETA(Hidden),
	Name        = 1 << 0    UMETA(DisplayName = "Name"),
	Thumbnail   = 1 << 1    UMETA(DisplayName = "Thumbnail"),
	Owner       = 1 << 2    UMETA(DisplayName = "Owner"),
	LastAccess  = 1 << 3    UMETA(DisplayName = "Last Access"),
	Keywords    = 1 << 4    UMETA(DisplayName = "Keywords"),
	Members     = 1 << 5    UMETA(DisplayName = "Members"),
	Tags        = 1 << 6    UMETA(DisplayName = "Tags"),
	All         = 0xFF      UMETA(DisplayName = "All Changes")
};
ENUM_CLASS_FLAGS(ESpaceInfoChangeFlags)
