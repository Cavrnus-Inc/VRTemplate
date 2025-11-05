// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusOrganizationInfo.h
 * @brief This file defines the FCavrnusOrganizationInfo structure used for storing information about an organization, in the future including available login methods and allowed feature sets.
 */

#pragma once

#include "CoreMinimal.h"
#include "CavrnusOrganizationInfo.generated.h"		// Always last

/**
 * @brief Structure to hold information about a Cavrnus space.
 *
 * The FCavrnusSpaceInfo structure contains the current name of the space, its unique ID, and a link to its thumbnail.
 * This information is used for building space selection UIs.
 */
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusOrganizationInfo
{
	GENERATED_BODY()

	/** @brief The unique ID of the space. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Server")
	FString DisplayName = "";

	/** @brief The current name of the space. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Server")
	FString Domain = "";

	FCavrnusOrganizationInfo(const FString& displayname, const FString& domain)
		: DisplayName(displayname), Domain(domain)
	{
	}

	FCavrnusOrganizationInfo() = default;

	/**
	 * @brief Equality operator to compare two FCavrnusSpaceInfo objects.
	 *
	 * @param other The other FCavrnusSpaceInfo object to compare with.
	 * @return true if all fields are equal, false otherwise.
	 */
	bool operator==(const FCavrnusOrganizationInfo& other) const
	{
		return DisplayName == other.DisplayName && Domain == other.Domain;
	}
};
