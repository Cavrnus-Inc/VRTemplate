// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusAuthentication.h
 * @brief This file defines the FCavrnusAuthentication structure used for storing authentication tokens.
 *
 * The FCavrnusAuthentication struct is used to represent the result of a successful login attempt.
 * It contains a token that can be stored locally to keep a user logged in. However, the CavrnusSpatialConnector
 * component already handles this functionality, so direct interaction with FCavrnusAuthentication is generally
 * unnecessary unless custom functionality is required.
 */

#pragma once

#include "CoreMinimal.h"

#include "CavrnusAuthentication.generated.h"		// Always last

/**
 * @brief Structure to hold authentication data.
 *
 * The FCavrnusAuthentication structure contains the token received from a successful login attempt.
 * This token can be used to keep a user logged in, although this is typically managed automatically by the CavrnusSpatialConnector component.
 */
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusAuthentication
{
	GENERATED_BODY()

	/** The authentication token received after a successful login. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Authorization")
	FString Token = "";

	/** The server received after a successful login, in the case of apikey and device-code logins */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Authorization")
	FString Server = "";

	/** The display name of the user logged in */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Authorization")
	FString DisplayName = "";

	/** The name of the organization of the user logged in */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Authorization")
	FString OrgName = "";

	/**
	 * @brief Constructor to initialize the FCavrnusAuthentication with a given token.
	 *
	 * @param Token The authentication token.
	 */
	FCavrnusAuthentication(const FString& Token, const FString& Server, const FString& userdisplayname, const FString& orgname)
		: Token(Token), Server(Server), DisplayName(userdisplayname), OrgName(orgname)
	{
	}

	/** Default constructor for FCavrnusAuthentication. */
	FCavrnusAuthentication() = default;
};
