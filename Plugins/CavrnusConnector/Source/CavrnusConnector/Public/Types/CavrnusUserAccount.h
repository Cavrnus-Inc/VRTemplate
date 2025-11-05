// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file FCavrnusUserAccount.h
 * @brief This file defines the FCavrnusUserAccount structure used for representing users present in a given space.
 *
 * The FCavrnusUserAccount struct represents a user account on the Cavrnus server. It is primarily used for inviting users to spaces.
 * The IsLocalUser field indicates if this user is the local user.
 * The structure includes well-known properties that are read-only and can be modified through the Admin Portal.
 */

#pragma once

#include "CoreMinimal.h"

#include "CavrnusUserAccount.generated.h"		// Always last

/**
* @brief Structure to hold information about a server account for a Cavrnus User.
*
* The FCavrnusUserAccount structure contains various fields representing the identity of a user account as defined on the server.  Note that one User Account can join the same space multiple times in different instances, resulting in multiple FCavrnusUsers.
*/
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusUserAccount
{
	GENERATED_BODY()

	/** Indicates if this user is the local user. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Users")
	bool IsLocalUser = false;

	/** The id of the account. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Users")
	FString AccountId = "";

	/** The email associated with this account. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Users")
	FString UserEmail = "";

	/** The unique ID of the user's connection to a space. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Users")
	FString UserProfilePic = "";

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Users")
	FString UserFirstName = "";

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Users")
	FString UserLastName = "";

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Users")
	FString UserDisplayName = "";

	/**
	 * @brief Equality operator for comparing two FCavrnusUserAccount objects.
	 *
	 * @param Other The other FCavrnusUserAccount object to compare with.
	 * @return True if both users have the same properties container name and user connection ID, false otherwise.
	 */
	bool operator==(const FCavrnusUserAccount& Other) const
	{
		return AccountId == Other.AccountId && UserEmail == Other.UserEmail && UserProfilePic == Other.UserProfilePic;
	}

	/** Default constructor for FCavrnusUserAccount. */
	FCavrnusUserAccount() = default;

	/**
	 * @brief Constructor to initialize the FCavrnusUserAccount.
	 */
	FCavrnusUserAccount(bool IsLocalUser, const FString& AccountId, const FString& UserEmail, const FString& UserProfilePic, const FString& UserFirstName, const FString& UserLastName, const FString& UserDisplayName)
		: IsLocalUser(IsLocalUser), AccountId(AccountId), UserEmail(UserEmail), UserProfilePic(UserProfilePic), UserFirstName(UserFirstName), UserLastName(UserLastName), UserDisplayName(UserDisplayName)
	{
	}
};
