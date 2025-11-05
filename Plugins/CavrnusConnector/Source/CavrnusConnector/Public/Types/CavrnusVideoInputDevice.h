// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusVideoInputDevice.h
 * @brief This file defines the FCavrnusVideoInputDevice structure used for representing available camera/video stream sources.
 *
 * The FCavrnusVideoInputDevice struct represents an available camera or video stream source for the user to select.
 * These devices are acquired by calling FetchVideoInputs and can be provided to UpdateVideoInput to change the video source.
 * The DeviceName field is useful for displaying this video source in a UI selector.
 */
#pragma once

#include "CoreMinimal.h"

#include "CavrnusVideoInputDevice.generated.h"		// Always last

 /**
  * @brief Structure to hold information about a video input device.
  *
  * The FCavrnusVideoInputDevice structure contains the device name and device ID for a video input device.
  * This information is used to allow users to select from available video input devices.
  */
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusVideoInputDevice
{
	GENERATED_BODY()

	/** The name of the video input device, useful for displaying in a UI selector. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Input")
	FString DeviceName = "";

	/** The unique ID of the video input device. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Input")
	FString DeviceId = "";

	/** Default constructor for FCavrnusVideoInputDevice. */
	FCavrnusVideoInputDevice() = default;

	/**
	 * @brief Constructor to initialize the FCavrnusVideoInputDevice with given values.
	 *
	 * @param DeviceName The name of the video input device.
	 * @param DeviceId The unique ID of the video input device.
	 */
	FCavrnusVideoInputDevice(const FString& DeviceName, const FString& DeviceId)
		: DeviceName(DeviceName), DeviceId(DeviceId)
	{
	}
};
