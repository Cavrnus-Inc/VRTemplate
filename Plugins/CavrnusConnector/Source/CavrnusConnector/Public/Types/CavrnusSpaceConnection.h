// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusSpaceConnection.h
 * @brief Declaration of the FCavrnusSpaceConnection struct, which has a single purpose - to
 *  define the identity of Space to order to match local and remote actions to a Space. Do not add
 *  additional run-time values here; those belong in FCavrnusSpaceConnectionInfo
 * 
 */

#pragma once

#include "CoreMinimal.h"
#include "CavrnusSpaceConnection.generated.h"		// Always last

/**
 * @brief Represents a connection to a Cavrnus Space.
 *
 * This struct holds information about the connection to a specific space in Cavrnus
 */
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusSpaceConnection
{
	GENERATED_BODY()

	/** Unique identifier for the space connection */
	UPROPERTY(BlueprintReadOnly, Category = "Cavrnus|Spaces")
	int SpaceConnectionId = -1;

	/**
	 * @brief Default constructor.
	 *
	 * Initializes the space connection with default values.
	 */
	FCavrnusSpaceConnection() = default;

	/**
	 * @brief Constructor with space connection ID.
	 *
	 * Initializes the space connection with the specified connection ID.
	 *
	 * @param SpaceConnectionId Unique identifier for the space connection.
	 */
	FCavrnusSpaceConnection(int SpaceConnectionId)
		: SpaceConnectionId(SpaceConnectionId)
	{
	}
};
