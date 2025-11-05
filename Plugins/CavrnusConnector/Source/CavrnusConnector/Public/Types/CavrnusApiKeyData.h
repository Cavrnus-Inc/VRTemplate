// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusApiKeyData.h
 * @brief This file defines the FCavrnusApiKeyData structure used for storing apikey information.
 */

#pragma once

#include "CoreMinimal.h"

#include "CavrnusApiKeyData.generated.h"		// Always last

/**
 * @brief Structure to hold authentication data.
 *
 * The FCavrnusAuthentication structure contains the token received from a successful login attempt.
 * This token can be used to keep a user logged in, although this is typically managed automatically by the CavrnusSpatialConnector component.
 */
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusApiKeyData
{
	GENERATED_BODY()

	/** The access key portion of the apikey */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Authorization")
	FString AccessKey = "";

	/** The access token (secret) part of the apikey */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Authorization")
	FString AccessToken = "";

	/**
	 * @brief Constructor to initialize the FCavrnusApiKeyData with its given data
	 */
	FCavrnusApiKeyData(const FString& accesskey, const FString& accesstoken)
		: AccessKey(accesskey), AccessToken(accesstoken)
	{
	}

	/** Default constructor for FCavrnusApiKeyData. */
	FCavrnusApiKeyData() = default;
};
