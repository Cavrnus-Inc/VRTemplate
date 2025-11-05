// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusSpaceConnectionInfo.h
 * @brief Declaration of the FCavrnusSpaceConnectionInfo struct, which contains run-time data
 * about a space's connection and state
 * 
 */

#pragma once

#include "CoreMinimal.h"
#include "CavrnusSpaceInfo.h"

#include "CavrnusSpaceConnectionInfo.generated.h"		// Always last

/**
 * @brief Represents a connection to a Cavrnus Space.
 *
 * This struct holds information about the connection to a specific space in Cavrnus,
 * including the connection ID, user connection ID, and container name.
 */
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusSpaceConnectionInfo
{
	GENERATED_BODY()

	/** Unique identifier for the space connection */
	UPROPERTY(BlueprintReadOnly, Category = "Cavrnus|Spaces")
	int SpaceConnectionId = -1;

	/** Identifier for the local user's connection */
	UPROPERTY(BlueprintReadOnly, Category = "Cavrnus|Spaces")
	FString LocalUserConnectionId = "";

	/** Name of the container associated with the local user */
	UPROPERTY(BlueprintReadOnly, Category = "Cavrnus|Spaces")
	FString LocalUserContainerName = "";

	/** Does the local user own this space */
	UPROPERTY(BlueprintReadOnly, Category = "Cavrnus|Spaces")
	bool SpaceOwnedByLocalUser = false;

	/** The current space's info */
	UPROPERTY(BlueprintReadOnly, Category = "Cavrnus|Spaces")
	FCavrnusSpaceInfo SpaceInfo;

	/**
	 * @brief Default constructor.
	 *
	 * Initializes the space connection with default values.
	 */
	FCavrnusSpaceConnectionInfo() = default;

	/**
	 * @brief Constructor with space connection ID.
	 *
	 * Initializes the space connection with the specified connection ID.
	 *
	 * @param SpaceConnectionId Unique identifier for the space connection.
	 */
	FCavrnusSpaceConnectionInfo(int SpaceConnectionId)
		: SpaceConnectionId(SpaceConnectionId)
	{
		SpaceInfo = FCavrnusSpaceInfo();
	}

	/**
	 * @brief Constructor with full initialization.
	 *
	 * Initializes the space connection with the specified connection ID, user connection ID, and container name.
	 *
	 * @param InSpaceConnectionId Unique identifier for the space connection.
	 * @param InLocalUserConnectionId Identifier for the local user's connection.
	 * @param InLocalUserContainerName Name of the container associated with the local user.
	 */
	FCavrnusSpaceConnectionInfo(int InSpaceConnectionId, const FString& InLocalUserConnectionId, const FString& InLocalUserContainerName, bool spaceOwnedByLocalUser, FCavrnusSpaceInfo spaceInfo)
		: SpaceConnectionId(InSpaceConnectionId), LocalUserConnectionId(InLocalUserConnectionId), LocalUserContainerName(InLocalUserContainerName), SpaceInfo(spaceInfo)
	{
		SpaceOwnedByLocalUser = spaceOwnedByLocalUser;
	}
};
