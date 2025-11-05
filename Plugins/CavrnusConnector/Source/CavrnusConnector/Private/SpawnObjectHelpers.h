// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusPropertiesContainer.h"
#include "Types/CavrnusUser.h"
#include "Types/CavrnusSpawnedObject.h"
#include <Components/SceneComponent.h>
#include <Materials/MaterialInstanceDynamic.h>

/**
 * @brief Helper class for spawning and setting up objects and users in the Cavrnus system.
 *
 * The SpawnObjectHelpers class provides utility functions for spawning and initializing
 * actors representing users and objects within the Cavrnus system. It includes methods
 * for handling the setup and configuration of spawned actors.
 */
class CAVRNUSCONNECTOR_API SpawnObjectHelpers
{
public:
	static SpawnObjectHelpers* GetSpawnObjectHelpers();

	static void Kill();

	/**
	 * @brief Spawns a user actor and sets it up with the provided user data.
	 *
	 * This method spawns an actor of the specified class in the given world and sets it up
	 * using the provided user data.
	 *
	 * @param World The world in which to spawn the actor.
	 * @param ActorClass The class of the actor to spawn.
	 * @param User The user data to set up the spawned actor.
	 * @return A pointer to the newly spawned actor.
	 */
	AActor* SpawnUserAndSetup(UWorld* World, TSubclassOf<AActor> ActorClass, const FCavrnusUser& User);

	/**
	 * @brief Spawns an object actor and sets it up with the provided object data.
	 *
	 * This method spawns an actor of the specified class in the given world and sets it up
	 * using the provided object data.
	 *
	 * @param World The world in which to spawn the actor.
	 * @param ActorClass The class of the actor to spawn.
	 * @param SpawnedObject The object data to set up the spawned actor.
	 * @return A pointer to the newly spawned actor.
	 */
	AActor* SpawnObjectAndSetup(UWorld* World, TSubclassOf<AActor> ActorClass, const FCavrnusSpawnedObject& SpawnedObject);

	/**
	 * @brief Resets the live hierarchy root name of the specified actor.
	 *
	 * This static method replaces the root name of the actor's live hierarchy with the provided replacement string.
	 *
	 * @param Actor The actor whose root name is to be reset.
	 * @param ReplacementString The string to replace the root name with.
	 */
	static void ResetLiveHierarchyRootName(AActor* Actor, const FString& ReplacementString);

	/**
	 * @brief Initializes the specified actor for use in the Cavrnus system.
	 *
	 * This method performs initial setup and configuration of the actor to ensure it is
	 * ready for use within the Cavrnus system.
	 *
	 * @param InActor The actor to initialize.
	 */
	void InitializeCavrnusActor(AActor* InActor);

private:
	static SpawnObjectHelpers* Instance;
	
	/** Map of spawned object actors and their corresponding object data. */
	TMap<AActor*, FCavrnusSpawnedObject> objectSpawns;

	/**
	 * @brief Handles the setup of a spawned object actor with the provided object data.
	 *
	 * This method configures the actor using the given object data.
	 *
	 * @param actor The spawned actor to set up.
	 * @param object The object data to use for setup.
	 */
	void HandleSpawnedObject(AActor* actor, const FCavrnusSpawnedObject& object);

	/**
	 * @brief Sets up property containers for the specified spawned actor.
	 *
	 * This method sets up the property containers for the actor using the provided container name.
	 *
	 * @param spawnedActor The spawned actor to set up.
	 * @param propsContainerName The name of the property container to use.
	 * @return A pointer to the set up actor.
	 */
	AActor* SetupObjectPropsContainers(AActor* spawnedActor, const FString& propsContainerName);

	/**
	 * @brief Replaces a placeholder in the properties container with a different string reference.
	 *
	 * This static method replaces occurrences of the specified placeholder in the properties
	 * container with the provided replacement string.
	 *
	 * @param PropertiesContainer The properties container to modify.
	 * @param Placeholder The placeholder string to replace.
	 * @param ReplacementString The string to replace the placeholder with.
	 *  sets to true if the container name was modified, false if not
	 */
	static void ReplacePlaceholderInPropertiesContainer(UCavrnusPropertiesContainer* PropertiesContainer, const FString& Placeholder, const FString& ReplacementString);
};
