// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusServerStatus.h
 * @brief This file defines the FCavrnusServerStatus structure used for passing information about the status of a given server endpoint.
 *
 * The FCavrnusSpaceInfo struct holds the most up-to-date information about a space. This can be used to create a
 * list of spaces, similar to the default Spaces List provided in Cavrnus. The structure contains fields for the
 * space's name, unique ID, and thumbnail link. These fields are primarily used for building space selection UIs.
 * To get a list of available CavrnusSpaceInfos, you can call FetchJoinableSpaces or BindJoinableSpaces.
 */

#pragma once

#include "CoreMinimal.h"
#include "CavrnusServerMessage.generated.h"		// Always last

UENUM(BlueprintType)
enum class EServerMessageCategory : uint8
{
	Log = 0,
	UserMessage = 1,
	CriticalServerMessage = 2,
	ProgramError = 3,
};

/**
 * @brief Structure to hold information about a Cavrnus space.
 *
 * The FCavrnusSpaceInfo structure contains the current name of the space, its unique ID, and a link to its thumbnail.
 * This information is used for building space selection UIs.
 */
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusServerMessage
{
	GENERATED_BODY()

	/** @brief If the server is live and domain valid. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Spaces")
	EServerMessageCategory Category = EServerMessageCategory::Log;

	/** @brief If not live, a message indicating the error. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Spaces")
	FString Message = "";

	FCavrnusServerMessage(const EServerMessageCategory& category, const FString& msg)
		: Category(category), Message(msg)
	{
	}

	FCavrnusServerMessage() = default;

	/**
	 * @brief Equality operator to compare two FCavrnusSpaceInfo objects.
	 *
	 * @param other The other FCavrnusSpaceInfo object to compare with.
	 * @return true if all fields are equal, false otherwise.
	 */
	bool operator==(const FCavrnusServerMessage& other) const
	{
		return Category == other.Category && Message == other.Message;
	}
};
