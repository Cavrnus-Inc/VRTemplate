// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusDeviceCodeData.h
 * @brief This file defines the FCavrnusDeviceCodeData structure used for storing information needed to authenticate using a temporary device code.
 */

#pragma once

#include "CoreMinimal.h"

#include "CavrnusDeviceCodeData.generated.h"		// Always last

/**
 * @brief Structure to hold authentication data.
 *
 * The CavrnusDeviceCodeData structure contains the data received by beginning a device-code based login. The process will normally open a browser window, which
 * will then authenticate the device code, causing future login attempts using the device code to succeed. In the case that the user device cannot open a browser window,
 * the UserCode field should be shown to the user, and they should be instructed to use it in the Cavrnus Console to authorize the device to be logged in.
 */
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusDeviceCodeData
{
	GENERATED_BODY()

	/** The authentication token received after a successful login. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Authorization")
	FString DeviceCode = "";

	/** The access key portion of the apikey */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Authorization")
	FString UserCode = "";

	/** The url to be navigated to to authenticate the device code. The CavrnusFunctionLibrary calls to initiate device-code login can
		be instructed to automatically open the url. If so then this url field should not be needed. If the device cannot open the browser to log in,
		then the url may be useful. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Authorization")
	FString VerificationUrl = "";

	/**
	 * @brief Constructor to initialize the FCavrnusDeviceCodeData with its given data
	 */
	FCavrnusDeviceCodeData(const FString& devicecode, const FString& usercode, const FString& verificationurl)
		: DeviceCode(devicecode), UserCode(usercode), VerificationUrl(verificationurl)
	{
	}

	/** Default constructor for FCavrnusDeviceCodeData. */
	FCavrnusDeviceCodeData() = default;
};
