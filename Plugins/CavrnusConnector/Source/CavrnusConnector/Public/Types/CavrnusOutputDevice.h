// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusOutputDevice.h
 * @brief This file defines the FCavrnusOutputDevice structure used for representing audio output devices in Cavrnus.
 *
 * The FCavrnusOutputDevice struct is used to represent an available speaker or audio output device for the user to select in the Cavrnus system.
 * This structure includes information such as the device name and device ID, which can be used to display available speakers in a user interface and select a specific device for audio output.
 * Note: This selection only affects the Cavrnus voice chat and does not impact the application's other sounds.
 */

#pragma once

#include "CoreMinimal.h"

#include "CavrnusOutputDevice.generated.h"		// Always last

/**
* @brief Structure to hold information about an audio output device in Cavrnus.
*
* The FCavrnusOutputDevice structure contains fields representing metadata about an available audio output device (speaker) in Cavrnus.
* This includes the device name and device ID, which can be used for selecting and displaying audio output options in the user interface.
*/
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusOutputDevice
{
	GENERATED_BODY()

	/** The name of the audio output device. Useful for displaying the device in a UI selector. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Input")
	FString DeviceName = "";

	/** The unique identifier for the audio output device. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Input")
	FString DeviceId = "";

	/** Default constructor for FCavrnusOutputDevice. */
	FCavrnusOutputDevice() = default;

	/**
	 * @brief Constructor to initialize the FCavrnusOutputDevice with given values.
	 *
	 * @param DeviceName The name of the audio output device.
	 * @param DeviceId The unique identifier for the audio output device.
	 */
	FCavrnusOutputDevice(const FString& DeviceName, const FString& DeviceId)
		: DeviceName(DeviceName), DeviceId(DeviceId)
	{
	}	
};
