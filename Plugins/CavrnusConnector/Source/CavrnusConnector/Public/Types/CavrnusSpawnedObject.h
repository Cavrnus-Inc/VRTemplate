// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusSpawnedObject.h
 * @brief This file defines the FCavrnusSpawnedObject structure used for representing object instances spawned in a Cavrnus space.
 *
 * The FCavrnusSpawnedObject struct represents an object instance that has been spawned using the SpawnObject function.
 * Each spawned instance has a unique Properties Container, represented by PropertiesContainerName, which allows individual property settings for each object.
 */

#pragma once

#include "CoreMinimal.h"
#include "CavrnusSpaceConnection.h"
#include <Engine/Texture2D.h>

#include "CavrnusSpawnedObject.generated.h"		// Always last

/**
* @brief Structure to hold information about a spawned object in a Cavrnus space.
*
* The FCavrnusSpawnedObject structure contains fields representing the state and identity of a spawned object in a Cavrnus space.
*/
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusSpawnedObject
{
	GENERATED_BODY()

	/** The connection to the space where this object is spawned. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Objects")
	FCavrnusSpaceConnection SpaceConnection = FCavrnusSpaceConnection();

	/** The name of the properties container associated with this object instance. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Objects")
	FString PropertiesContainerName = "";

	/** The Actor instance corresponding to this creation. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Objects")
	AActor* SpawnedActorInstance = nullptr;

	/**
	 * @brief Constructor to initialize the FCavrnusSpawnedObject with given values.
	 *
	 * @param PropertiesContainerName The name of the properties container associated with this object instance.
	 * @param CreationOpId The unique ID of the operation that created this object instance.
	 * @param Actor The actor associated with this spawned object.
	 */
	FCavrnusSpawnedObject() = default;
};
