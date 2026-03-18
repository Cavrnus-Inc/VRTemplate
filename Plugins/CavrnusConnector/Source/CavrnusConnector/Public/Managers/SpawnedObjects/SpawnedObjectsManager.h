#pragma once

#include "CoreMinimal.h"
#include "Types/CavrnusSpawnedObject.h"
#include "Types/PropertiesContainer.h"
#include "Abstract/FileImporter/CavrnusBaseLoader_Abstract.h"
#include "CavrnusImportDelegates.h"
#include "Managers/CavrnusService.h"
#include "UI/Systems/Messages/ToastMessages/Progress/CavrnusProgressToastMessageWidget.h"
#include "Engine/Engine.h"
#include "Managers/SpawnedObjects/CavrnusSpawnPropertyValue.h"
#include "SpawnedObjectsManager.generated.h"

class UCavrnusPendingSpawnObject;
class UCavrnusSpawnableRegistryDataAsset;

// Forward declarations for types used in static functions
struct FCavrnusSpaceConnection;

/**
 * @brief Manages the lifecycle of spawned objects in the Cavrnus system.
 *
 * The SpawnedObjectsManager class is responsible for dispatching load requests
 * to the appropriate loader, and for tracking/unregistering the resulting actors.
 */
 UCLASS()
class CAVRNUSCONNECTOR_API USpawnedObjectsManager : public UCavrnusService
{
	GENERATED_BODY()
public:
    virtual void Initialize() override;
    virtual void Dispose() override;

    /**
     * @brief Dispatches a load request into the Cavrnus system.
     *
     * Instantiates a transient loader (UCavrnusBaseLoader subclass) to process
     * the provided spawned object data. The loader may asynchronously create
     * one or more actors or assets in the given world, and will report progress
     * and completion through its delegates. The manager tracks the resulting
     * spawned actors once the loader signals completion.
     *
     * @param SpawnedObject The spawned object data received from the network.
     * @param Identifier The well-known object identifier used to select the loader.
     * @param World The world context in which any resulting actors should be created.
     * @return A pointer to the loader instance handling this request, or nullptr if no loader was found.
     */
    UCavrnusBaseLoader_Abstract* RegisterSpawnedObjectAsync(const FCavrnusSpawnedObject& SpawnedObject, const FString& Identifier, UWorld* World);

    AActor* RegisterSpawnedObject(const FCavrnusSpawnedObject& SpawnedObject, const FString& Identifier, UWorld* world);

    /**
     * @brief Unregisters a spawned object from the Cavrnus system.
     *
     * Removes the actor corresponding to the provided spawned object from the world
     * and unregisters it from the internal tracking map.
     */
    void UnregisterSpawnedObject(const FCavrnusSpawnedObject& SpawnedObject, UWorld* World);

    /** Destroy all tracked spawned actors and clear the registry. */
    void Clear();

    /**
     * @brief Register Spawnable Object class type.
     * @param Identifier The name to match when SpawnObject is called.
     * @param ClassType The AActor class that will be instantiated.
     */
    void RegisterSpawnableObjectType(const FString& Identifier, TSubclassOf<AActor> ClassType);

    /**
     * @brief Unregister Spawnable Object class type.
     * @param Identifier The name to match when SpawnObject is called.
     */
    void UnregisterSpawnableObjectType(const FString& Identifier);
    void UpdateProgressToast(const FPropertiesContainer& Key, const FCavrnusImportStatus& Status);
    void CloseProgressToast(const FPropertiesContainer& Key, float Delay = 0.5f);

    /**
     * @brief Spawns an actor of the specified class in the Cavrnus space.
     * 
     * This function matches Unreal Engine's SpawnActorFromClass API. It creates a journal
     * command and sets up a pending spawn object that waits for the Transform property
     * before spawning the actor.
     * 
     * Note: This is NOT a UFUNCTION because UFUNCTIONS cannot accept TMap parameters.
     * The K2Node builds the call manually.
     * 
     * @param SpaceConnection The space connection where the object will be spawned
     * @param ActorClass The class of actor to spawn
     * @param SpawnTransform Optional initial transform (may be overridden by journal property)
     * @param CollisionHandlingOverride Optional collision handling override
     * @param DataAsset Optional DataAsset to use for validation (defaults to main one)
     * @param ExposeOnSpawnValues Map of ExposeOnSpawn property values from Blueprint pins (keyed by property name)
     * @return The spawned actor, or nullptr if spawning failed
     */
    static AActor* CavrnusSpawnActorFromClass(
        FCavrnusSpaceConnection SpaceConnection,
        TSubclassOf<AActor> ActorClass,
        const FTransform& SpawnTransform = FTransform(),
        ESpawnActorCollisionHandlingMethod CollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined,
        AActor* Owner = nullptr,
        UCavrnusSpawnableRegistryDataAsset* DataAsset = nullptr,
        const TMap<FString, FCavrnusSpawnPropertyValue>& ExposeOnSpawnValues = TMap<FString, FCavrnusSpawnPropertyValue>()
    );

    /**
     * @brief Helper function that takes an array of ExposeOnSpawn values and converts to map.
     * This is used by the K2Node since building a TMap in Blueprint bytecode is complex.
     * 
     * Note: This is a UFUNCTION so it can be called from Blueprint/K2Node.
     * TArray of USTRUCTs is supported in UFUNCTIONS.
     * 
     * @param SpaceConnection The space connection where the object will be spawned
     * @param ActorClass The class of actor to spawn
     * @param SpawnTransform Optional initial transform
     * @param CollisionHandlingOverride Optional collision handling override
     * @param DataAsset Optional DataAsset to use for validation
     * @param ExposeOnSpawnValuesArray Array of ExposeOnSpawn property values from Blueprint pins
     * @return The spawned actor, or nullptr if spawning failed
     */
    UFUNCTION(BlueprintCallable, Category = "Cavrnus|Objects",
        meta = (ToolTip = "Spawns an actor with ExposeOnSpawn property values from an array", CallInEditor = "false", BlueprintInternalUseOnly = "true"))
    static AActor* CavrnusSpawnActorFromClassWithArray(
        FCavrnusSpaceConnection SpaceConnection,
        TSubclassOf<AActor> ActorClass,
        const FTransform& SpawnTransform,
        ESpawnActorCollisionHandlingMethod CollisionHandlingOverride,
        AActor* Owner,
        UCavrnusSpawnableRegistryDataAsset* DataAsset,
        const TArray<FCavrnusSpawnPropertyValue>& ExposeOnSpawnValuesArray
    );

    /**
     * @brief Spawns an actor using a well-known object identifier.
     * 
     * This function looks up the actor class or static mesh from the DataAsset using
     * the wellKnownObjectId, then spawns it following the same flow as CavrnusSpawnActorFromClass.
     * 
     * @param SpaceConnection The space connection where the object will be spawned
     * @param WellKnownObjectId The well-known object identifier to lookup in DataAsset
     * @param SpawnTransform Optional initial transform (may be overridden by journal property)
     * @param CollisionHandlingOverride Optional collision handling override
     * @param DataAsset Optional DataAsset to use for lookup (defaults to main one)
     * @param ExposeOnSpawnValues Map of ExposeOnSpawn property values from Blueprint pins
     * @return The spawned actor, or nullptr if spawning is pending or lookup failed
     */
    static AActor* CavrnusSpawnActorById(
        FCavrnusSpaceConnection SpaceConnection,
        const FString& WellKnownObjectId,
        const FTransform& SpawnTransform = FTransform(),
        ESpawnActorCollisionHandlingMethod CollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined,
        AActor* Owner = nullptr,
        UCavrnusSpawnableRegistryDataAsset* DataAsset = nullptr,
        const TMap<FString, FCavrnusSpawnPropertyValue>& ExposeOnSpawnValues = TMap<FString, FCavrnusSpawnPropertyValue>()
    );

    /**
     * @brief Spawns an actor by ID with ExposeOnSpawn properties from Blueprint pins.
     * 
     * This is a UFUNCTION wrapper that accepts an array of ExposeOnSpawn property values
     * and converts them to a map for the main CavrnusSpawnActorById function.
     * 
     * @param SpaceConnection The space connection where the object will be spawned
     * @param WellKnownObjectId The well-known object identifier to lookup in DataAsset
     * @param SpawnTransform Initial transform
     * @param CollisionHandlingOverride Collision handling override
     * @param DataAsset Optional DataAsset to use for lookup (defaults to main one)
     * @param ExposeOnSpawnValuesArray Array of ExposeOnSpawn property values from Blueprint pins
     * @return The spawned actor, or nullptr if spawning is pending or lookup failed
     */
    UFUNCTION(BlueprintCallable, Category = "Cavrnus|Objects",
        meta = (ToolTip = "Spawns an actor by ID in the Cavrnus space with ExposeOnSpawn properties.", 
                ShortToolTip = "Spawn actor by ID with ExposeOnSpawn", 
                BlueprintInternalUseOnly = "true"))
    static AActor* CavrnusSpawnActorByIdWithArray(
        FCavrnusSpaceConnection SpaceConnection,
        const FString& WellKnownObjectId,
        const FTransform& SpawnTransform,
        ESpawnActorCollisionHandlingMethod CollisionHandlingOverride,
        AActor* Owner,
        UCavrnusSpawnableRegistryDataAsset* DataAsset,
        const TArray<FCavrnusSpawnPropertyValue>& ExposeOnSpawnValuesArray
    );

    /**
     * @brief Constructs a UObject instance from the specified class in the Cavrnus space.
     * 
     * This function matches Unreal Engine's ConstructObjectFromClass API. It creates a journal
     * command and constructs the object immediately (synchronous). For remote construction
     * (from journal), use OnObjectCreation callback which defers until ExposeOnSpawn properties arrive.
     * 
     * @param SpaceConnection The space connection where the object will be constructed
     * @param ObjectClass The class of object to construct
     * @param DataAsset Optional DataAsset to use for validation (defaults to main one)
     * @param ExposeOnSpawnValues Map of ExposeOnSpawn property values from Blueprint pins
     * @return The constructed object, or nullptr if construction failed
     */
    static UObject* CavrnusConstructObjectFromClass(
        FCavrnusSpaceConnection SpaceConnection,
        TSubclassOf<UObject> ObjectClass,
        UObject* Outer = nullptr,
        UCavrnusSpawnableRegistryDataAsset* DataAsset = nullptr,
        const TMap<FString, FCavrnusSpawnPropertyValue>& ExposeOnSpawnValues = TMap<FString, FCavrnusSpawnPropertyValue>()
    );

    /**
     * @brief Constructs an object from class with ExposeOnSpawn properties from Blueprint pins.
     * 
     * This is a UFUNCTION wrapper that accepts an array of ExposeOnSpawn property values
     * and converts them to a map for the main CavrnusConstructObjectFromClass function.
     * 
     * @param SpaceConnection The space connection where the object will be constructed
     * @param ObjectClass The class of object to construct
     * @param DataAsset Optional DataAsset to use for validation (defaults to main one)
     * @param ExposeOnSpawnValuesArray Array of ExposeOnSpawn property values from Blueprint pins
     * @return The constructed object, or nullptr if construction failed
     */
    UFUNCTION(BlueprintCallable, Category = "Cavrnus|Objects",
        meta = (ToolTip = "Constructs an object from class in the Cavrnus space with ExposeOnSpawn properties.", 
                ShortToolTip = "Construct object from class with ExposeOnSpawn", 
                BlueprintInternalUseOnly = "true"))
    static UObject* CavrnusConstructObjectFromClassWithArray(
        FCavrnusSpaceConnection SpaceConnection,
        TSubclassOf<UObject> ObjectClass,
        UObject* Outer,
        UCavrnusSpawnableRegistryDataAsset* DataAsset,
        const TArray<FCavrnusSpawnPropertyValue>& ExposeOnSpawnValuesArray
    );

    /**
     * @brief Registers a custom pending spawn object class for a specific well-known object ID.
     * 
     * Modules can use this to register custom handlers (e.g., for bookmarks that wait
     * for metadata before spawning).
     * 
     * @param WellKnownObjectId The well-known object identifier
     * @param PendingSpawnClass The custom pending spawn object class to use
     */
    void RegisterCustomSpawnObjectClass(const FString& WellKnownObjectId, TSubclassOf<UCavrnusPendingSpawnObject> PendingSpawnClass);

    /**
     * @brief Unregisters a custom pending spawn object class.
     * 
     * @param WellKnownObjectId The well-known object identifier
     */
    void UnregisterCustomSpawnObjectClass(const FString& WellKnownObjectId);

    /**
     * @brief Registers a DataAsset for spawnable object lookups.
     * 
     * Modules (like CVT) can register their own DataAssets to extend the list
     * of spawnable objects. DataAssets are checked in registration order after
     * the default DataAsset. Safe to call at any time, even before SpawnedObjectsManager
     * is fully initialized.
     * 
     * @param DataAsset The DataAsset to register
     */
    void RegisterSpawnableDataAsset(UCavrnusSpawnableRegistryDataAsset* DataAsset);

    /**
     * @brief Unregisters a DataAsset from spawnable object lookups.
     * 
     * @param DataAsset The DataAsset to unregister
     */
    void UnregisterSpawnableDataAsset(UCavrnusSpawnableRegistryDataAsset* DataAsset);

    /**
     * @brief Registers a spawned actor with the manager.
     * 
     * Called by pending spawn objects when they successfully spawn an actor.
     * 
     * @param SpawnedObject The spawned object data
     * @param SpawnedActor The spawned actor instance
     */
    void RegisterSpawnedActor(const FCavrnusSpawnedObject& SpawnedObject, AActor* SpawnedActor);

    /**
     * @brief Finds the actor class for a well-known object ID in a DataAsset.
     * 
     * Matches Unreal Engine's SpawnActor behavior - only accepts AActor classes.
     * 
     * @param WellKnownObjectId The well-known object identifier
     * @param OptionalDataAsset Optional DataAsset to search (if nullptr, uses default)
     * @param OutActorClass Output parameter for the actor class (if found)
     * @param OutStaticMesh Unused parameter, kept for signature compatibility
     * @return true if found, false otherwise
     */
    bool FindSpawnableClassOrMesh(
        const FString& WellKnownObjectId,
        UCavrnusSpawnableRegistryDataAsset* OptionalDataAsset,
        TSubclassOf<AActor>& OutActorClass,
        UStaticMesh*& OutStaticMesh
    );

    /**
     * @brief Helper function to create object in journal and register locally.
     * 
     * This encapsulates the pattern of sending a createObject journal command
     * and registering the object locally via HandleSpaceObjectAdded() to prevent
     * duplicates when the server response arrives. Matches the pattern used in
     * the old SpawnObject function.
     * 
     * @param SpaceConnection The space connection where the object is created
     * @param WellKnownObjectId The well-known identifier for the object type
     * @param InstanceId The unique instance identifier (property container name)
     */
    static void CreateAndRegisterObject(
        const FCavrnusSpaceConnection& SpaceConnection,
        const FString& WellKnownObjectId,
        const FString& InstanceId
    );

    /**
     * @brief Finds the WellKnownObjectId (key) for a given actor class by reverse lookup.
     * 
     * Searches through DataAsset entries to find which entry has the matching actor class.
     * This is used for CavrnusSpawnActorFromClass to get the key from the actor class.
     * 
     * @param ActorClass The actor class to find the key for
     * @param OptionalDataAsset Optional DataAsset to check first (highest priority)
     * @param OutWellKnownObjectId The found WellKnownObjectId (key) if successful
     * @return true if found, false otherwise
     */
    bool FindWellKnownObjectIdForActorClass(
        TSubclassOf<AActor> ActorClass,
        UCavrnusSpawnableRegistryDataAsset* OptionalDataAsset,
        FString& OutWellKnownObjectId
    );

    /**
     * @brief Finds the UObject class for a well-known object ID in a DataAsset.
     * 
     * @param WellKnownObjectId The well-known object identifier
     * @param OptionalDataAsset Optional DataAsset to search (if nullptr, uses default)
     * @param OutObjectClass Output parameter for the object class (if found)
     * @return true if found, false otherwise
     */
    bool FindSpawnableObjectClass(
        const FString& WellKnownObjectId,
        UCavrnusSpawnableRegistryDataAsset* OptionalDataAsset,
        TSubclassOf<UObject>& OutObjectClass
    );

    /**
     * @brief Finds the WellKnownObjectId (key) for a given UObject class by reverse lookup.
     * 
     * Searches through DataAsset entries to find which entry has the matching object class.
     * This is used for CavrnusConstructObjectFromClass to get the key from the object class.
     * 
     * @param ObjectClass The object class to find the key for
     * @param OptionalDataAsset Optional DataAsset to check first (highest priority)
     * @param OutWellKnownObjectId The found WellKnownObjectId (key) if successful
     * @return true if found, false otherwise
     */
    bool FindWellKnownObjectIdForObjectClass(
        TSubclassOf<UObject> ObjectClass,
        UCavrnusSpawnableRegistryDataAsset* OptionalDataAsset,
        FString& OutWellKnownObjectId
    );

    /**
     * @brief Creates a pending construct object with a UObject class (for remote construction).
     * 
     * Uses the factory registry to determine which class to instantiate.
     * 
     * @param SpawnedObject The spawned object data
     * @param WellKnownObjectId The well-known object identifier
     * @param ObjectClass The object class to construct
     * @return The created pending spawn object
     */
    UCavrnusPendingSpawnObject* CreatePendingConstructObject(
        const FCavrnusSpawnedObject& SpawnedObject,
        const FString& WellKnownObjectId,
        TSubclassOf<UObject> ObjectClass
    );

    /**
     * @brief Creates a pending spawn object with an actor class.
     * 
     * Uses the factory registry to determine which class to instantiate.
     * 
     * @param SpawnedObject The spawned object data
     * @param WellKnownObjectId The well-known object identifier
     * @param ActorClass The actor class to spawn
     * @param CollisionHandling The collision handling method
     * @return The created pending spawn object
     */
    UCavrnusPendingSpawnObject* CreatePendingSpawnObjectWithActorClass(
        const FCavrnusSpawnedObject& SpawnedObject,
        const FString& WellKnownObjectId,
        TSubclassOf<AActor> ActorClass,
        ESpawnActorCollisionHandlingMethod CollisionHandling
    );

     const FString GetCacheFolder() {
         return TEXT("CavrnusCache");
     }
private:
    static USpawnedObjectsManager* Instance;

    UPROPERTY()
    /** Map of spawnable actors class types, keyed by a unique identifier string. */
    TMap<FString, TSubclassOf<AActor>> SpawnableIdentifiers;

    UPROPERTY()
    /** Map of spawned actors, keyed by a unique property container from the Journal. */
    TMap<FPropertiesContainer, TWeakObjectPtr<AActor>> spawnedActors;

    /** Track InFlight Loaders */
    UPROPERTY()
    TMap<FPropertiesContainer, UCavrnusBaseLoader_Abstract*> ActiveLoaders;

    /** Tracking for Toast messages */
    TMap<FPropertiesContainer, TWeakObjectPtr<UCavrnusProgressToastMessageWidget>> ActiveProgressToasts;

    /** Factory registry for custom pending spawn object classes */
    UPROPERTY()
    TMap<FString, TSubclassOf<UCavrnusPendingSpawnObject>> CustomSpawnObjectClasses;

    /** Tracking for pending spawn objects */
    UPROPERTY()
    TMap<FPropertiesContainer, TWeakObjectPtr<UCavrnusPendingSpawnObject>> PendingSpawns;

    /** Registry of additional DataAssets for spawnable object lookups (modules can register their own) */
    UPROPERTY()
    TArray<TWeakObjectPtr<UCavrnusSpawnableRegistryDataAsset>> RegisteredSpawnableDataAssets;
};
