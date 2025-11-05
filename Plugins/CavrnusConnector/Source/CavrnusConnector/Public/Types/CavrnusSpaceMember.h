// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file FCavrnusSpaceMember.h
 * @brief This file defines the FCavrnusSpaceMember structure used for representing user memberships for a given space.
 *
 * The FCavrnusSpaceMember struct represents a membership for a user in a specific space, on the Cavrnus server.
 * The IsLocalUser field indicates if this user is the local user.
 * The structure includes well-known properties that are read-only and can be modified through the Admin Portal.
 */

#pragma once

#include "CoreMinimal.h"

#include "CavrnusUserAccount.h"

#include "CavrnusSpaceMember.generated.h"		// Always last

/**
* @brief Structure to hold information about a server account for a Cavrnus User.
*
* The FCavrnusUserAccount structure contains various fields representing the identity of a user account as defined on the server.  Note that one User Account can join the same space multiple times in different instances, resulting in multiple FCavrnusUsers.
*/
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusSpaceMember
{
	GENERATED_BODY()

	/** Indicates if this user is the owner of the space. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Member")
	bool IsOwner = false;

	/** Indicates if this user has ever joined the space. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Member")
	bool HasJoined = false;

	/** The id of the account. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Member")
	FCavrnusUserAccount User;

	/**
	 * @brief Equality operator for comparing two FCavrnusUserAccount objects.
	 *
	 * @param Other The other FCavrnusUserAccount object to compare with.
	 * @return True if both users have the same properties container name and user connection ID, false otherwise.
	 */
	bool operator==(const FCavrnusSpaceMember& Other) const
	{
		return IsOwner == Other.IsOwner && HasJoined == Other.HasJoined && User == Other.User;
	}

	/** Default constructor for FCavrnusUserAccount. */
	FCavrnusSpaceMember() = default;

	/**
	 * @brief Constructor to initialize the FCavrnusUserAccount.
	 */
	FCavrnusSpaceMember(bool isOwner, bool HasJoined, const FCavrnusUserAccount& user)
		: IsOwner(isOwner), HasJoined(HasJoined), User(user)
	{
	}
};
