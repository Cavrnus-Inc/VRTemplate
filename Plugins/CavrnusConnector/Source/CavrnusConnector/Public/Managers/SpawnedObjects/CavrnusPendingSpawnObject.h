// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Types/CavrnusSpawnedObject.h"
#include "Types/CavrnusBinding.h"
#include "Managers/SpawnedObjects/CavrnusSpawnPropertyHelpers.h"
#include "Engine/EngineTypes.h"
#include "UObject/TextProperty.h"
#include "CavrnusPendingSpawnObject.generated.h"

class USpawnedObjectsManager;

/**
 * @brief Base class for managing pending spawns that wait for properties before creating actors.
 *
 * This class handles the two-stage spawning process: first receiving the createObject command
 * from the journal, then waiting for required properties (like Transform) before spawning the actor.
 * Custom subclasses can override Initialize() to wait for different properties (e.g., bookmark metadata).
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusPendingSpawnObject : public UObject
{
	GENERATED_BODY()

public:
	UCavrnusPendingSpawnObject(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * @brief Initializes the pending spawn object and starts listening for properties.
	 * 
	 * Base implementation binds to Transform property. Custom subclasses should override
	 * this to bind to their specific required properties.
	 */
	virtual void Initialize();

	/**
	 * @brief Sets up the pending spawn with an actor class.
	 * 
	 * @param InSpawnedObject The spawned object data from the journal
	 * @param InWellKnownObjectId The well-known object identifier
	 * @param InActorClass The actor class to spawn
	 * @param InCollisionHandling The collision handling method for spawning
	 * @param InManager The manager instance that owns this pending spawn
	 */
	void SetupWithActorClass(
		const FCavrnusSpawnedObject& InSpawnedObject,
		const FString& InWellKnownObjectId,
		TSubclassOf<AActor> InActorClass,
		ESpawnActorCollisionHandlingMethod InCollisionHandling,
		USpawnedObjectsManager* InManager
	);


    /**
     * @brief Sets up the pending spawn with a UObject class (for non-Actor objects).
     * 
     * @param InSpawnedObject The spawned object data from the journal
     * @param InWellKnownObjectId The well-known object identifier
     * @param InObjectClass The object class to construct
     * @param InManager The manager instance that owns this pending spawn
     */
    void SetupWithObjectClass(
        const FCavrnusSpawnedObject& InSpawnedObject,
        const FString& InWellKnownObjectId,
        TSubclassOf<UObject> InObjectClass,
        USpawnedObjectsManager* InManager
    );

protected:
	/**
	 * @brief Called when the Transform property arrives. Spawns the actor.
	 * 
	 * @param Transform The transform value from the journal
	 * @param ContainerName The properties container name
	 * @param PropertyName The property name (should be "Transform")
	 */
	UFUNCTION()
	void OnTransformReceived(FTransform Transform, FString ContainerName, FString PropertyName);

	/**
	 * @brief Spawns the actor with the given transform.
	 * 
	 * @param Transform The transform to spawn at
	 */
	void SpawnActorWithTransform(const FTransform& Transform);

	/**
	 * @brief Constructs the UObject (for non-Actor objects, no transform needed).
	 */
	void ConstructObject();

	/** The spawned object data from the journal */
	UPROPERTY()
	FCavrnusSpawnedObject SpawnedObject;

	/** The well-known object identifier */
	UPROPERTY()
	FString WellKnownObjectId;

    /** The actor class to spawn (set when using SetupWithActorClass) */
    UPROPERTY()
    TSubclassOf<AActor> ActorClass;


    /** The object class to construct (set when using SetupWithObjectClass) */
    UPROPERTY()
    TSubclassOf<UObject> ObjectClass;

	/** The collision handling method */
	UPROPERTY()
	ESpawnActorCollisionHandlingMethod CollisionHandling;

	/** Reference to the manager that owns this pending spawn */
	UPROPERTY()
	USpawnedObjectsManager* Manager;

	/** Binding for the Transform property */
	UPROPERTY()
	UCavrnusBinding* TransformBinding;

	/** ExposeOnSpawn properties found in the actor class */
	UPROPERTY()
	TArray<FCavrnusExposeOnSpawnProperty> ExposeOnSpawnProperties;

	/** Bindings for ExposeOnSpawn properties */
	UPROPERTY()
	TMap<FString, UCavrnusBinding*> ExposeOnSpawnBindings;

	/** Received property values for ExposeOnSpawn properties */
	TMap<FString, Cavrnus::FPropertyValue> ReceivedPropertyValues;

	/** The received transform (stored until all properties are ready) */
	FTransform ReceivedTransform;

	/** Whether the transform has been received */
	bool bTransformReceived;

	/** Whether this pending spawn has already spawned an actor */
	bool bHasSpawned;

	/**
	 * @brief Called when an ExposeOnSpawn property is received.
	 * 
	 * @param PropertyName The name of the property
	 * @param Value The property value
	 */
	void OnExposeOnSpawnPropertyReceived(const FString& PropertyName, const Cavrnus::FPropertyValue& Value);

	/**
	 * @brief Checks if all required properties (Transform + ExposeOnSpawn) have been received.
	 * 
	 * @return True if all properties are received and ready to spawn
	 */
	bool AreAllPropertiesReceived() const;

    /**
     * @brief Applies received ExposeOnSpawn property values to the spawned actor or constructed object.
     * 
     * @param SpawnedActor The actor to apply properties to (if spawning an actor)
     */
    void ApplyExposeOnSpawnProperties(AActor* SpawnedActor);

    /**
     * @brief Applies received ExposeOnSpawn property values to the constructed object.
     * 
     * @param ConstructedObject The object to apply properties to (if constructing a UObject)
     */
    void ApplyExposeOnSpawnProperties(UObject* ConstructedObject);
};

