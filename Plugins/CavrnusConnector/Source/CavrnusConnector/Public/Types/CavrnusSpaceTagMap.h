// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusSpaceTagMap.h
 * @brief Defines FCavrnusSpaceTagMap, a UHT-safe wrapper for TMap<FString, FString> used in space tag operations.
 */

#pragma once

#include "CoreMinimal.h"

#include "CavrnusSpaceTagMap.generated.h"		// Always last

/**
 * @brief UHT-safe wrapper for a string-to-string tag map.
 *
 * TMap cannot be used directly in DECLARE_DYNAMIC_DELEGATE params,
 * so this struct wraps it for Blueprint compatibility.
 */
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusSpaceTagMap
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|SpaceTags|Advanced")
	TMap<FString, FString> Tags;

	FCavrnusSpaceTagMap() = default;

	FCavrnusSpaceTagMap(const TMap<FString, FString>& InTags)
		: Tags(InTags)
	{
	}
};
