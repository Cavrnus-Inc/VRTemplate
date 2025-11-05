// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusInputDevice.h
 * @brief This file defines the FCavrnusInputDevice structure used for representing audio input devices (microphones) in Cavrnus.
 *
 * The FCavrnusInputDevice struct is used to represent an available microphone for the user to select in the Cavrnus system.
 * This structure includes information such as the device name and device ID, which can be used to display available microphones in a user interface and select a specific device for audio input.
 * The DeviceName field is particularly useful for displaying this microphone in a UI selector.
 */

#pragma once

#include "CoreMinimal.h"

#include "CavrnusInputDevice.generated.h"		// Always last

/**
* @brief Structure to hold information about an audio input device (microphone) in Cavrnus.
*
* The FCavrnusInputDevice structure contains fields representing metadata about an available microphone in Cavrnus.
* This includes the device name and device ID, which can be used for selecting and displaying audio input options in the user interface.
*/
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusInputDevice
{
	GENERATED_BODY()

	/** The name of the audio input device. Useful for displaying the device in a UI selector. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Input")
	FString DeviceName = "";

	/** The unique identifier for the audio input device. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Input")
	FString DeviceId = "";

	/** Default constructor for FCavrnusInputDevice. */
	FCavrnusInputDevice() = default;

	/**
	 * @brief Constructor to initialize the FCavrnusInputDevice with given values.
	 *
	 * @param DeviceName The name of the audio input device.
	 * @param DeviceId The unique identifier for the audio input device.
	 */
	FCavrnusInputDevice(const FString& DeviceName, const FString& DeviceId)
		: DeviceName(DeviceName), DeviceId(DeviceId)
	{
	}
};
