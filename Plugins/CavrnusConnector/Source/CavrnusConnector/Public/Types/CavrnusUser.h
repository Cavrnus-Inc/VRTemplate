// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusUser.h
 * @brief This file defines the FCavrnusUser structure used for representing users present in a given space.
 *
 * The FCavrnusUser struct represents a user in a Cavrnus space. It is primarily used for building user lists in a UI.
 * The IsLocalUser field indicates if this user is the local user.
 * The PropertiesContainerName field can be used to interact with the user's properties.
 * The structure includes well-known properties that are read-only and can be modified through the Admin Portal.
 */

#pragma once

#include "CoreMinimal.h"
#include "Types/CavrnusSpaceConnection.h"
#include "Engine/Texture2D.h"

#include "CavrnusUser.generated.h"		// Always last

/**
* @brief Structure to hold information about a user in a Cavrnus space.
*
* The FCavrnusUser structure contains various fields representing the state and identity of a user in a Cavrnus space.
*/
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusUser
{
	GENERATED_BODY()

	/** Indicates if this user is the local user. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Users")
	bool IsLocalUser = false;

	/** The name of the properties container associated with this user. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Users")
	FString PropertiesContainerName = "";

	/** The unique ID of the user's connection to a space. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Users")
	FString UserConnectionId = "";

	/** The connection to the space that this user is part of. */
	UPROPERTY(BlueprintReadonly, Category = "Cavrnus|Users")
	FCavrnusSpaceConnection SpaceConn = FCavrnusSpaceConnection();

	/**
	 * @brief Equality operator for comparing two FCavrnusUser objects.
	 *
	 * @param Other The other FCavrnusUser object to compare with.
	 * @return True if both users have the same properties container name and user connection ID, false otherwise.
	 */
	bool operator==(const FCavrnusUser& Other) const
	{
		return PropertiesContainerName == Other.PropertiesContainerName && UserConnectionId == Other.UserConnectionId;
	}

	/** Default constructor for FCavrnusUser. */
	FCavrnusUser() = default;

	/**
	 * @brief Constructor to initialize the FCavrnusUser with given values.
	 *
	 * @param IsLocalUser Indicates if this user is the local user.
	 * @param PropertiesContainerName The name of the properties container associated with this user.
	 * @param UserConnectionId The unique ID of the user's connection to a space.
	 * @param SpaceConn The connection to the space that this user is part of.
	 */
	FCavrnusUser(bool IsLocalUser, const FString& PropertiesContainerName, const FString& UserConnectionId, const FCavrnusSpaceConnection& SpaceConn)
		: IsLocalUser(IsLocalUser), PropertiesContainerName(PropertiesContainerName), UserConnectionId(UserConnectionId), SpaceConn(SpaceConn)
	{
	}
};
