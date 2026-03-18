// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Managers/SpawnedObjects/SpawnedObjectsManager.h"
#include "CavrnusConnectorModule.h"

#include "UObject/Package.h"
#include "Misc/PackageName.h"
#include "CavrnusFunctionLibrary.h"
#include "Managers/SpawnedObjects/SpawnObjectHelpers.h"
#include "Managers/SpawnedObjects/CavrnusPendingSpawnObject.h"
#include "Managers/SpawnedObjects/CavrnusSpawnPropertyHelpers.h"
#include "UObject/UnrealType.h"
#include "Abstract/FileImporter/CavrnusBaseLoader_Abstract.h"
#include "Abstract/FileImporter/CavrnusLoaderRegistry_Abstract.h"
#include "UI/CavrnusUI.h"
#include "UI/CavrnusUISystems.h"
#include "UI/Systems/Messages/CavrnusScopedMessages.h"
#include "UI/Systems/Messages/ToastMessages/Info/CavrnusInfoToastMessageWidget.h"
#include "UI/Systems/Messages/ToastMessages/CavrnusToastMessageUISystem.h"
#include "AssetManager/DataAssets/CavrnusSpawnableRegistryDataAsset.h"
#include "AssetManager/CavrnusDataAssetManager.h"
#include "Core/Subsystems/CavrnusSubsystem.h"
#include "Translation/CavrnusProtoTranslation.h"
#include "RelayModel/CavrnusRelayModel.h"
#include "Core/Contexts/CavrnusRuntimeContext.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Kismet/GameplayStatics.h"
#include "FlagComponents/CavrnusSpawnedObjectFlag.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"

inline UCavrnusBaseLoader_Abstract* CreateLoaderSafe(const FString& Identifier)
{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 6
    return FCavrnusLoaderRegistry_Abstract::Get().CreateMatchingLoader(Identifier, GetTransientPackage());
#else
    return FCavrnusLoaderRegistry_Abstract::Get().CreateMatchingLoader(Identifier, (UObject*)GetTransientPackage());
#endif
}

void USpawnedObjectsManager::Initialize()
{
    const FString CacheFolder = GetCacheFolder();
    const FString PhysicalPath = FPaths::Combine(FPaths::ProjectSavedDir(), CacheFolder);
    const FString MountPoint = TEXT("/") + CacheFolder + TEXT("/");

    FPackageName::RegisterMountPoint(MountPoint, PhysicalPath);

    Super::Initialize();
}

UCavrnusBaseLoader_Abstract* USpawnedObjectsManager::RegisterSpawnedObjectAsync(
    const FCavrnusSpawnedObject& SpawnedObject,
    const FString& Identifier,
    UWorld* World)
{
    UCavrnusBaseLoader_Abstract* Loader = CreateLoaderSafe(Identifier);

    if (!Loader)
    {
        UE_LOG(LogCavrnusConnector, Error,
            TEXT("No loader registered for Identifier %s"), *Identifier);
        return nullptr;
    }

    const FPropertiesContainer Key(SpawnedObject.PropertiesContainerName);
    ActiveLoaders.Add(Key, Loader);

    Loader->OnStatusUpdateNative.AddLambda([this](const FCavrnusImportStatus& Status)
    {
        const FPropertiesContainer Key(Status.FileKey);
        UpdateProgressToast(Key, Status);
    });

    Loader->OnCompleteNative.AddLambda([this, Key, SpawnedObject](const FCavrnusImportStatus& FinalStatus)
    {
        const FPropertiesContainer ToastKey(FinalStatus.FileKey);
        ActiveLoaders.Remove(Key);

        CloseProgressToast(ToastKey);

        // Show completion info toast
        UCavrnusInfoToastMessageWidget* InfoToast =
            UCavrnusUI::Get()->Messages()->Toast()->CreateAutoClose<UCavrnusInfoToastMessageWidget>(5.0f);

        if (InfoToast)
        {
            InfoToast->SetPrimaryText(FinalStatus.bSuccess ? TEXT("Import Complete") : TEXT("Import Failed"));
            InfoToast->SetSecondaryText(FinalStatus.StatusMessage);
            InfoToast->SetType(FinalStatus.bSuccess ? ECavrnusInfoToastMessageEnum::Success
                : ECavrnusInfoToastMessageEnum::Error);
        }

        if (FinalStatus.bSuccess && FinalStatus.ImportedAssets.Num() > 0)
        {
            if (AActor* Spawned = Cast<AActor>(FinalStatus.ImportedAssets[0]))
            {
                spawnedActors.Add(Key, Spawned);
            }
        }
    });

    Loader->StartLoad(SpawnedObject, World);
    return Loader;
}


AActor* USpawnedObjectsManager::RegisterSpawnedObject(const FCavrnusSpawnedObject& SpawnedObject, const FString& Identifier, UWorld* world)
{
	if (!SpawnableIdentifiers.Contains(Identifier))
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("Could not find spawnable object with Unique ID %s in the Cavrnus Spatial Connector"), *Identifier);
		return nullptr;
	}

	auto actor = SpawnObjectHelpers::GetSpawnObjectHelpers()->SpawnObjectAndSetup(world, SpawnableIdentifiers[Identifier], SpawnedObject);

	spawnedActors.Add(FPropertiesContainer(SpawnedObject.PropertiesContainerName), actor);

	return actor;
}

void USpawnedObjectsManager::UnregisterSpawnedObject(const FCavrnusSpawnedObject& SpawnedObject, UWorld* World)
{
    const FPropertiesContainer Key(SpawnedObject.PropertiesContainerName);

    // Case 1: Actor already spawned
    if (TWeakObjectPtr<AActor>* FoundActor = spawnedActors.Find(Key))
    {
        if (AActor* Actor = FoundActor->Get())
        {
            Actor->Destroy();
            UE_LOG(LogCavrnusConnector, Log,
                TEXT("Destroyed actor for container %s"), *SpawnedObject.PropertiesContainerName);
        }
        else
        {
            UE_LOG(LogCavrnusConnector, Warning,
                TEXT("Actor for container %s was already invalid"), *SpawnedObject.PropertiesContainerName);
        }

        spawnedActors.Remove(Key);
        return;
    }

    // Case 2: Loader still in flight
    if (UCavrnusBaseLoader_Abstract** FoundLoaderPtr = ActiveLoaders.Find(Key))
    {
        if (UCavrnusBaseLoader_Abstract* Loader = *FoundLoaderPtr)
        {
            Loader->CancelLoad();
            UE_LOG(LogCavrnusConnector, Log,
                TEXT("Cancelled loader for container %s"), *SpawnedObject.PropertiesContainerName);
        }

        ActiveLoaders.Remove(Key);
        return;
    }

    // Case 3: Nothing found
    UE_LOG(LogCavrnusConnector, Error,
        TEXT("Failed to unregister, no actor or loader found for container %s"),
        *SpawnedObject.PropertiesContainerName);
}


void USpawnedObjectsManager::Clear()
{
	for (auto a : spawnedActors)
	{
		if (a.Value.IsValid())
			a.Value->Destroy();
	}
	spawnedActors.Empty();

    // Cancel any in-flight loaders
    for (auto& Pair : ActiveLoaders)
    {
        if (UCavrnusBaseLoader_Abstract* Loader = Pair.Value)
        {
            Loader->CancelLoad();
        }
    }
    ActiveLoaders.Empty();

    // Clean up pending spawns
    for (auto& Pair : PendingSpawns)
    {
        if (UCavrnusPendingSpawnObject* PendingSpawn = Pair.Value.Get())
        {
            if (IsValid(PendingSpawn))
            {
                PendingSpawn->MarkAsGarbage();
            }
        }
    }
    PendingSpawns.Empty();
}

void USpawnedObjectsManager::RegisterSpawnableObjectType(const FString& Identifier, TSubclassOf<AActor> ClassType)
{
	if (SpawnableIdentifiers.Contains(Identifier))
	{
		SpawnableIdentifiers[Identifier] = ClassType;
	}
	else
	{
		SpawnableIdentifiers.Add(Identifier, ClassType);
	}
}

void USpawnedObjectsManager::UnregisterSpawnableObjectType(const FString& Identifier)
{
	if (SpawnableIdentifiers.Contains(Identifier))
	{
		SpawnableIdentifiers.Remove(Identifier);
	}
}

void USpawnedObjectsManager::UpdateProgressToast(const FPropertiesContainer& Key, const FCavrnusImportStatus& Status)
{
    UCavrnusProgressToastMessageWidget* Toast;
    TWeakObjectPtr<UCavrnusProgressToastMessageWidget> Existing = ActiveProgressToasts.FindRef(Key);

    if (Existing.IsValid())
    {
        Toast = Existing.Get();
    }
    else
    {
        Toast = UCavrnusUI::Get()->Messages()->Toast()->Create<UCavrnusProgressToastMessageWidget>();
        ActiveProgressToasts.Add(Key, Toast);
    }

    if (Toast)
    {
        Toast->SetPrimaryText(Status.StatusMessage);
        Toast->SetSecondaryText(Status.SecondaryMessage);
        Toast->SetProgress(Status.Progress);
    }
}

void USpawnedObjectsManager::CloseProgressToast(const FPropertiesContainer& Key, float Delay)
{
    if (TWeakObjectPtr<UCavrnusProgressToastMessageWidget> Existing = ActiveProgressToasts.FindRef(Key); Existing.IsValid())
    {
        //Existing->CloseWithDelay(Delay);
        Existing->Close();
        ActiveProgressToasts.Remove(Key);
    }
}

void USpawnedObjectsManager::Dispose()
{
    Super::Dispose();
    Clear();

    // Clean up pending spawns
    for (auto& Pair : PendingSpawns)
    {
        if (UCavrnusPendingSpawnObject* PendingSpawn = Pair.Value.Get())
        {
            if (IsValid(PendingSpawn))
            {
                PendingSpawn->MarkAsGarbage();
            }
        }
    }
    PendingSpawns.Empty();
    CustomSpawnObjectClasses.Empty();
    RegisteredSpawnableDataAssets.Empty();
}

AActor* USpawnedObjectsManager::CavrnusSpawnActorFromClass(
    FCavrnusSpaceConnection SpaceConnection,
    TSubclassOf<AActor> ActorClass,
    const FTransform& SpawnTransform,
    ESpawnActorCollisionHandlingMethod CollisionHandlingOverride,
    AActor* Owner,
    UCavrnusSpawnableRegistryDataAsset* DataAsset,
    const TMap<FString, FCavrnusSpawnPropertyValue>& ExposeOnSpawnValues)
{
    // Log all ExposeOnSpawn values from the TMap
    UE_LOG(LogCavrnusConnector, Log, TEXT("CavrnusSpawnActorFromClass: Received %d ExposeOnSpawn values in TMap"), 
        ExposeOnSpawnValues.Num());
    
    for (const auto& ValuePair : ExposeOnSpawnValues)
    {
        const FString& PropertyName = ValuePair.Key;
        const FCavrnusSpawnPropertyValue& Value = ValuePair.Value;
        
        FString ValueStr;
        switch (Value.PropertyType)
        {
        case ECavrnusSpawnPropertyType::Bool:
            ValueStr = Value.BoolValue ? TEXT("true") : TEXT("false");
            break;
        case ECavrnusSpawnPropertyType::Float:
            ValueStr = FString::SanitizeFloat(Value.FloatValue);
            break;
        case ECavrnusSpawnPropertyType::String:
            ValueStr = FString::Printf(TEXT("\"%s\""), *Value.StringValue);
            break;
        case ECavrnusSpawnPropertyType::Vector:
            ValueStr = Value.VectorValue.ToString();
            break;
        case ECavrnusSpawnPropertyType::Transform:
            ValueStr = Value.TransformValue.ToString();
            break;
        case ECavrnusSpawnPropertyType::Color:
            ValueStr = Value.ColorValue.ToString();
            break;
        case ECavrnusSpawnPropertyType::Struct:
            ValueStr = FString::Printf(TEXT("Struct(\"%s\")"), *Value.StringValue);
            break;
        default:
            ValueStr = TEXT("Unknown");
            break;
        }
        
        // UE_LOG(LogCavrnusConnector, Log, TEXT("  - Property: '%s', Type: %d, Value: %s"), 
        //    *PropertyName, static_cast<int32>(Value.PropertyType), *ValueStr);
    }

    if (!ActorClass)
    {
        UE_LOG(LogCavrnusConnector, Error, TEXT("CavrnusSpawnActorFromClass: ActorClass is null"));
        return nullptr;
    }

    UCavrnusSubsystem* Subsystem = UCavrnusSubsystem::Get();
    if (!Subsystem || !Subsystem->RuntimeContext)
    {
        UE_LOG(LogCavrnusConnector, Error, TEXT("CavrnusSpawnActorFromClass: Subsystem or RuntimeContext is invalid"));
        return nullptr;
    }

    USpawnedObjectsManager* Manager = Subsystem->RuntimeContext->Get<USpawnedObjectsManager>();
    if (!Manager)
    {
        UE_LOG(LogCavrnusConnector, Error, TEXT("CavrnusSpawnActorFromClass: SpawnedObjectsManager is invalid"));
        return nullptr;
    }

    // Validate that the actor class exists in one of the Spawnable DataAssets
    // We need to find the WellKnownObjectId (key) for this actor class by reverse lookup
    // This matches the pattern used in OnObjectCreation where we have the key and find the class
    FString WellKnownObjectId;
    if (!Manager->FindWellKnownObjectIdForActorClass(ActorClass, DataAsset, WellKnownObjectId))
    {
        UE_LOG(LogCavrnusConnector, Error, 
            TEXT("CavrnusSpawnActorFromClass: Actor class %s is not found in any SpawnableActorsDataAsset. Cannot spawn."), 
            *ActorClass->GetName());
        return nullptr;
    }

    // Get world for spawning
    UWorld* World = nullptr;
    if (GEngine && GEngine->GameViewport)
    {
        if (UGameViewportClient* ViewportClient = GEngine->GameViewport.Get())
        {
            World = ViewportClient->GetWorld();
        }
    }

    if (!World)
    {
        // Fallback: try to get from world contexts
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }

    if (!World)
    {
        UE_LOG(LogCavrnusConnector, Error, TEXT("CavrnusSpawnActorFromClass: World is invalid"));
        return nullptr;
    }

    // Create instance ID
    FString InstanceId = Cavrnus::CavrnusProtoTranslation::CreateTransientId();

    // Create object in journal and register locally
    CreateAndRegisterObject(SpaceConnection, WellKnownObjectId, InstanceId);

    // Create FCavrnusSpawnedObject
    FCavrnusSpawnedObject SpawnedObject;
    SpawnedObject.SpaceConnection = SpaceConnection;
    SpawnedObject.PropertiesContainerName = InstanceId;

    // Use provided collision handling or default
    ESpawnActorCollisionHandlingMethod FinalCollisionHandling = 
        (CollisionHandlingOverride != ESpawnActorCollisionHandlingMethod::Undefined) 
        ? CollisionHandlingOverride 
        : ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // LOCAL CALL: Spawn immediately with provided transform
    TSubclassOf<AActor> FinalActorClass = ActorClass;
    
    // Spawn the actor with the provided transform using SpawnObjectHelpers
    AActor* SpawnedActor = SpawnObjectHelpers::GetSpawnObjectHelpers()->SpawnObjectAndSetupWithTransform(
        World,
        FinalActorClass,
        SpawnTransform,
        SpawnedObject,
        FinalCollisionHandling,
        Owner
    );

    if (!SpawnedActor)
    {
        UE_LOG(LogCavrnusConnector, Error, TEXT("CavrnusSpawnActorFromClass: Failed to spawn actor"));
        return nullptr;
    }

	FPropertyPostOptions PostOptions;
	PostOptions.Smoothed = false;
    // Post transform property to journal (local call - we post, not wait)
    UCavrnusFunctionLibrary::PostTransformPropertyUpdate(
        SpaceConnection,
        InstanceId,
        TEXT("Transform"),
        SpawnTransform,
        PostOptions
    );

    // Get ExposeOnSpawn properties for validation
    TArray<FCavrnusExposeOnSpawnProperty> ExposeOnSpawnProps = 
        FCavrnusSpawnPropertyHelpers::GetExposeOnSpawnProperties(ActorClass);
    
    // First, process all values from the map (pin values)
    for (const auto& PinValuePair : ExposeOnSpawnValues)
    {
        const FString& PropertyName = PinValuePair.Key;
        const FCavrnusSpawnPropertyValue& PinValue = PinValuePair.Value;
        
        // Find the corresponding property
        FProperty* Prop = nullptr;
        for (const FCavrnusExposeOnSpawnProperty& ExposeProp : ExposeOnSpawnProps)
        {
            if (ExposeProp.PropertyName == PropertyName && ExposeProp.Property)
            {
                Prop = ExposeProp.Property;
                break;
            }
        }
        
        if (!Prop)
        {
            UE_LOG(LogCavrnusConnector, Warning, TEXT("CavrnusSpawnActorFromClass: Property '%s' from pin not found in actor class"), 
                *PropertyName);
            continue;
        }

        // UE_LOG(LogCavrnusConnector, Log, TEXT("CavrnusSpawnActorFromClass: Processing pin value for property '%s' (Type: %d)"), 
        //    *PropertyName, static_cast<int32>(PinValue.PropertyType));

        // Apply the value to the spawned actor
        void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(SpawnedActor);
        if (ValuePtr)
        {
            bool bApplied = PinValue.ApplyToProperty(Prop, ValuePtr);
            if (!bApplied)
            {
                UE_LOG(LogCavrnusConnector, Warning, TEXT("CavrnusSpawnActorFromClass: Failed to apply pin value to property '%s'"), 
                    *PropertyName);
            }
        }
        else
        {
            UE_LOG(LogCavrnusConnector, Warning, TEXT("CavrnusSpawnActorFromClass: Could not get value pointer for property '%s'"), 
                *PropertyName);
        }
    }
    
    // Now post all ExposeOnSpawn properties to the journal (use pin values if available, otherwise CDO)
    for (const FCavrnusExposeOnSpawnProperty& ExposeProp : ExposeOnSpawnProps)
    {
        if (!ExposeProp.Property)
        {
            continue;
        }

        // Check if we have a value from the pin (ExposeOnSpawnValues map)
        // If not, fall back to reading from the spawned actor (which now has pin values applied)
        Cavrnus::FPropertyValue CavrnusValue;

        if (const FCavrnusSpawnPropertyValue* PinValue = ExposeOnSpawnValues.Find(ExposeProp.PropertyName))
        {
            // Use value from pin
            CavrnusValue = PinValue->ToCavrnusPropertyValue();
        }
        else
        {
            // Fall back to reading from spawned actor (CDO values or previously applied pin values)
            UE_LOG(LogCavrnusConnector, Log, TEXT("CavrnusSpawnActorFromClass: No pin value for property '%s', reading from actor"), 
                *ExposeProp.PropertyName);
            
            void* ValuePtr = ExposeProp.Property->ContainerPtrToValuePtr<void>(SpawnedActor);
            if (!ValuePtr)
            {
                continue;
            }

            // Convert to Cavrnus value
            CavrnusValue = 
                FCavrnusSpawnPropertyHelpers::PropertyToCavrnusValue(ExposeProp.Property, ValuePtr);
        }

        // Post to journal based on property type
        FProperty* Prop = ExposeProp.Property;
        if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop))
        {
            if (CavrnusValue.PropType == Cavrnus::FPropertyValue::PropertyType::Bool)
            {
                UCavrnusFunctionLibrary::PostBoolPropertyUpdate(
                    SpaceConnection,
                    InstanceId,
                    ExposeProp.PropertyName,
                    CavrnusValue.BoolValue
                );
            }
        }
        else if (FNumericProperty* NumericProp = CastField<FNumericProperty>(Prop))
        {
            if (CavrnusValue.PropType == Cavrnus::FPropertyValue::PropertyType::Float)
            {
                UCavrnusFunctionLibrary::PostFloatPropertyUpdate(
                    SpaceConnection,
                    InstanceId,
                    ExposeProp.PropertyName,
                    CavrnusValue.FloatValue
                );
            }
        }
        else if (FStrProperty* StrProp = CastField<FStrProperty>(Prop))
        {
            if (CavrnusValue.PropType == Cavrnus::FPropertyValue::PropertyType::String)
            {
                UCavrnusFunctionLibrary::PostStringPropertyUpdate(
                    SpaceConnection,
                    InstanceId,
                    ExposeProp.PropertyName,
                    CavrnusValue.StringValue
                );
            }
        }
        else if (FTextProperty* TextProp = CastField<FTextProperty>(Prop))
        {
            // Text properties are sent as strings
            if (CavrnusValue.PropType == Cavrnus::FPropertyValue::PropertyType::String)
            {
                UCavrnusFunctionLibrary::PostStringPropertyUpdate(
                    SpaceConnection,
                    InstanceId,
                    ExposeProp.PropertyName,
                    CavrnusValue.StringValue
                );
            }
        }
        else if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
        {
            UScriptStruct* Struct = StructProp->Struct;
            if (Struct)
            {
                // Handle native struct types
                if (Struct == TBaseStructure<FVector>::Get() || Struct == TBaseStructure<FVector4>::Get())
                {
                    if (CavrnusValue.PropType == Cavrnus::FPropertyValue::PropertyType::Vector)
                    {
                        UCavrnusFunctionLibrary::PostVectorPropertyUpdate(
                            SpaceConnection,
                            InstanceId,
                            ExposeProp.PropertyName,
                            CavrnusValue.VectorValue
                        );
                    }
                }
                else if (Struct == TBaseStructure<FTransform>::Get())
                {
                    if (CavrnusValue.PropType == Cavrnus::FPropertyValue::PropertyType::Transform)
                    {
                        UCavrnusFunctionLibrary::PostTransformPropertyUpdate(
                            SpaceConnection,
                            InstanceId,
                            ExposeProp.PropertyName,
                            CavrnusValue.TransformValue,
                            PostOptions
                        );
                    }
                }
                else if (Struct == TBaseStructure<FLinearColor>::Get() || Struct == TBaseStructure<FColor>::Get())
                {
                    if (CavrnusValue.PropType == Cavrnus::FPropertyValue::PropertyType::Color)
                    {
                        UCavrnusFunctionLibrary::PostColorPropertyUpdate(
                            SpaceConnection,
                            InstanceId,
                            ExposeProp.PropertyName,
                            CavrnusValue.ColorValue
                        );
                    }
                }
                else
                {
                    // For arbitrary structs, send as string
                    if (CavrnusValue.PropType == Cavrnus::FPropertyValue::PropertyType::String)
                    {
                        UCavrnusFunctionLibrary::PostStringPropertyUpdate(
                            SpaceConnection,
                            InstanceId,
                            ExposeProp.PropertyName,
                            CavrnusValue.StringValue
                        );
                    }
                }
            }
        }
    }

    // Register with manager
    Manager->RegisterSpawnedActor(SpawnedObject, SpawnedActor);

    // Add to SpawnedObjects map to prevent duplicates when journal response arrives
    Cavrnus::SpacePropertyModel* PropertyModel = 
        Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePropertyModel(SpaceConnection);
    if (PropertyModel)
    {
        SpawnedObject.SpawnedActorInstance = SpawnedActor;
        PropertyModel->SpawnedObjects.Add(InstanceId, SpawnedObject);
    }

    // Return the spawned actor immediately
    return SpawnedActor;
}

AActor* USpawnedObjectsManager::CavrnusSpawnActorFromClassWithArray(
    FCavrnusSpaceConnection SpaceConnection,
    TSubclassOf<AActor> ActorClass,
    const FTransform& SpawnTransform,
    ESpawnActorCollisionHandlingMethod CollisionHandlingOverride,
    AActor* Owner,
    UCavrnusSpawnableRegistryDataAsset* DataAsset,
    const TArray<FCavrnusSpawnPropertyValue>& ExposeOnSpawnValuesArray)
{
    // Handle default values (UFUNCTIONS don't support default parameters)
    FTransform FinalTransform = SpawnTransform;
    ESpawnActorCollisionHandlingMethod FinalCollisionHandling = 
        (CollisionHandlingOverride != ESpawnActorCollisionHandlingMethod::Undefined) 
        ? CollisionHandlingOverride 
        : ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // Convert array to map
    TMap<FString, FCavrnusSpawnPropertyValue> ExposeOnSpawnValues;
    for (const FCavrnusSpawnPropertyValue& Value : ExposeOnSpawnValuesArray)
    {
        if (!Value.PropertyName.IsEmpty())
        {
            ExposeOnSpawnValues.Add(Value.PropertyName, Value);
        }
    }
    // Call the main function with the map
    return CavrnusSpawnActorFromClass(
        SpaceConnection,
        ActorClass,
        FinalTransform,
        FinalCollisionHandling,
        Owner,
        DataAsset,
        ExposeOnSpawnValues
    );
}

AActor* USpawnedObjectsManager::CavrnusSpawnActorById(
    FCavrnusSpaceConnection SpaceConnection,
    const FString& WellKnownObjectId,
    const FTransform& SpawnTransform,
    ESpawnActorCollisionHandlingMethod CollisionHandlingOverride,
    AActor* Owner,
    UCavrnusSpawnableRegistryDataAsset* DataAsset,
    const TMap<FString, FCavrnusSpawnPropertyValue>& ExposeOnSpawnValues)
{
    // Log all ExposeOnSpawn values from the TMap
    
    for (const auto& ValuePair : ExposeOnSpawnValues)
    {
        const FString& PropertyName = ValuePair.Key;
        const FCavrnusSpawnPropertyValue& Value = ValuePair.Value;
        
        FString ValueStr;
        switch (Value.PropertyType)
        {
        case ECavrnusSpawnPropertyType::Bool:
            ValueStr = Value.BoolValue ? TEXT("true") : TEXT("false");
            break;
        case ECavrnusSpawnPropertyType::Float:
            ValueStr = FString::SanitizeFloat(Value.FloatValue);
            break;
        case ECavrnusSpawnPropertyType::String:
            ValueStr = FString::Printf(TEXT("\"%s\""), *Value.StringValue);
            break;
        case ECavrnusSpawnPropertyType::Vector:
            ValueStr = Value.VectorValue.ToString();
            break;
        case ECavrnusSpawnPropertyType::Transform:
            ValueStr = Value.TransformValue.ToString();
            break;
        case ECavrnusSpawnPropertyType::Color:
            ValueStr = Value.ColorValue.ToString();
            break;
        case ECavrnusSpawnPropertyType::Struct:
            ValueStr = FString::Printf(TEXT("Struct(\"%s\")"), *Value.StringValue);
            break;
        default:
            ValueStr = TEXT("Unknown");
            break;
        }
        
    }

    if (WellKnownObjectId.IsEmpty())
    {
        UE_LOG(LogCavrnusConnector, Error, TEXT("CavrnusSpawnActorById: WellKnownObjectId is empty"));
        return nullptr;
    }

    UCavrnusSubsystem* Subsystem = UCavrnusSubsystem::Get();
    if (!Subsystem || !Subsystem->RuntimeContext)
    {
        UE_LOG(LogCavrnusConnector, Error, TEXT("CavrnusSpawnActorById: Subsystem or RuntimeContext is invalid"));
        return nullptr;
    }

    USpawnedObjectsManager* Manager = Subsystem->RuntimeContext->Get<USpawnedObjectsManager>();
    if (!Manager)
    {
        UE_LOG(LogCavrnusConnector, Error, TEXT("CavrnusSpawnActorById: SpawnedObjectsManager is invalid"));
        return nullptr;
    }

    // Lookup actor class from DataAsset (matching Unreal Engine's SpawnActor - only AActor classes)
    TSubclassOf<AActor> ActorClass = nullptr;
    UStaticMesh* StaticMesh = nullptr; // Unused, kept for function signature compatibility

    if (!Manager->FindSpawnableClassOrMesh(WellKnownObjectId, DataAsset, ActorClass, StaticMesh))
    {
        UE_LOG(LogCavrnusConnector, Error, TEXT("CavrnusSpawnActorById: Could not find ActorClass for WellKnownObjectId %s"), *WellKnownObjectId);
        return nullptr;
    }

    if (!ActorClass)
    {
        UE_LOG(LogCavrnusConnector, Error, TEXT("CavrnusSpawnActorById: No valid ActorClass found for WellKnownObjectId %s"), *WellKnownObjectId);
        return nullptr;
    }

    // Create instance ID
    FString InstanceId = Cavrnus::CavrnusProtoTranslation::CreateTransientId();

    // Create object in journal and register locally
    CreateAndRegisterObject(SpaceConnection, WellKnownObjectId, InstanceId);

    // Create FCavrnusSpawnedObject (we'll use this directly, not from the map)
    FCavrnusSpawnedObject SpawnedObject;
    SpawnedObject.SpaceConnection = SpaceConnection;
    SpawnedObject.PropertiesContainerName = InstanceId;

    // Use provided collision handling or default
    ESpawnActorCollisionHandlingMethod FinalCollisionHandling = 
        (CollisionHandlingOverride != ESpawnActorCollisionHandlingMethod::Undefined) 
        ? CollisionHandlingOverride 
        : ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // Get world for spawning
    UWorld* World = nullptr;
    if (GEngine && GEngine->GameViewport)
    {
        if (UGameViewportClient* ViewportClient = GEngine->GameViewport.Get())
        {
            World = ViewportClient->GetWorld();
        }
    }

    if (!World)
    {
        // Fallback: try to get from world contexts
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }

    if (!World)
    {
        UE_LOG(LogCavrnusConnector, Error, TEXT("CavrnusSpawnActorById: World is invalid"));
        return nullptr;
    }

    // LOCAL CALL: Spawn immediately with provided transform
    TSubclassOf<AActor> FinalActorClass = ActorClass;

    // Spawn the actor with the provided transform using SpawnObjectHelpers
    AActor* SpawnedActor = SpawnObjectHelpers::GetSpawnObjectHelpers()->SpawnObjectAndSetupWithTransform(
        World,
        FinalActorClass,
        SpawnTransform,
        SpawnedObject,
        FinalCollisionHandling,
        Owner
    );

    if (!SpawnedActor)
    {
        UE_LOG(LogCavrnusConnector, Error, TEXT("CavrnusSpawnActorById: Failed to spawn actor"));
        return nullptr;
    }

	FPropertyPostOptions PostOptions;
    PostOptions.Smoothed = false;
    // Post transform property to journal (local call - we post, not wait)
    UCavrnusFunctionLibrary::PostTransformPropertyUpdate(
        SpaceConnection,
        InstanceId,
        TEXT("Transform"),
        SpawnTransform,
        PostOptions
    );

    // Get ExposeOnSpawn properties for validation
    TArray<FCavrnusExposeOnSpawnProperty> ExposeOnSpawnProps = 
        FCavrnusSpawnPropertyHelpers::GetExposeOnSpawnProperties(FinalActorClass);
    
    // First, process all values from the map (pin values) and apply them to the spawned actor
    for (const auto& PinValuePair : ExposeOnSpawnValues)
    {
        const FString& PropertyName = PinValuePair.Key;
        const FCavrnusSpawnPropertyValue& PinValue = PinValuePair.Value;
        
        // Find the corresponding property
        FProperty* Prop = nullptr;
        for (const FCavrnusExposeOnSpawnProperty& ExposeProp : ExposeOnSpawnProps)
        {
            if (ExposeProp.PropertyName == PropertyName && ExposeProp.Property)
            {
                Prop = ExposeProp.Property;
                break;
            }
        }
        
        if (!Prop)
        {
            UE_LOG(LogCavrnusConnector, Warning, TEXT("CavrnusSpawnActorById: Property '%s' from pin not found in actor class"), 
                *PropertyName);
            continue;
        }

        // Apply the value to the spawned actor
        void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(SpawnedActor);
        if (ValuePtr)
        {
            bool bApplied = PinValue.ApplyToProperty(Prop, ValuePtr);
            if (!bApplied)
            {
                UE_LOG(LogCavrnusConnector, Warning, TEXT("CavrnusSpawnActorById: Failed to apply pin value to property '%s'"), 
                    *PropertyName);
            }
        }
        else
        {
            UE_LOG(LogCavrnusConnector, Warning, TEXT("CavrnusSpawnActorById: Could not get value pointer for property '%s'"), 
                *PropertyName);
        }
    }
    
    // Now post all ExposeOnSpawn properties to the journal (use pin values if available, otherwise CDO)
    for (const FCavrnusExposeOnSpawnProperty& ExposeProp : ExposeOnSpawnProps)
    {
        if (!ExposeProp.Property)
        {
            continue;
        }

        // Check if we have a value from the pin (ExposeOnSpawnValues map)
        // If not, fall back to reading from the spawned actor (which now has pin values applied)
        Cavrnus::FPropertyValue CavrnusValue;

        if (const FCavrnusSpawnPropertyValue* PinValue = ExposeOnSpawnValues.Find(ExposeProp.PropertyName))
        {
            // Use value from pin
            CavrnusValue = PinValue->ToCavrnusPropertyValue();
        }
        else
        {
            // Fall back to reading from spawned actor (CDO values or previously applied pin values)
            UE_LOG(LogCavrnusConnector, Log, TEXT("CavrnusSpawnActorById: No pin value for property '%s', reading from actor"), 
                *ExposeProp.PropertyName);
            
            void* ValuePtr = ExposeProp.Property->ContainerPtrToValuePtr<void>(SpawnedActor);
            if (!ValuePtr)
            {
                continue;
            }

            // Convert to Cavrnus value
            CavrnusValue = 
                FCavrnusSpawnPropertyHelpers::PropertyToCavrnusValue(ExposeProp.Property, ValuePtr);
        }

        // Post to journal based on property type
        FProperty* Prop = ExposeProp.Property;
        if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop))
        {
            if (CavrnusValue.PropType == Cavrnus::FPropertyValue::PropertyType::Bool)
            {
                UCavrnusFunctionLibrary::PostBoolPropertyUpdate(
                    SpaceConnection,
                    InstanceId,
                    ExposeProp.PropertyName,
                    CavrnusValue.BoolValue
                );
            }
        }
        else if (FNumericProperty* NumericProp = CastField<FNumericProperty>(Prop))
        {
            if (CavrnusValue.PropType == Cavrnus::FPropertyValue::PropertyType::Float)
            {
                UCavrnusFunctionLibrary::PostFloatPropertyUpdate(
                    SpaceConnection,
                    InstanceId,
                    ExposeProp.PropertyName,
                    CavrnusValue.FloatValue
                );
            }
        }
        else if (FStrProperty* StrProp = CastField<FStrProperty>(Prop))
        {
            if (CavrnusValue.PropType == Cavrnus::FPropertyValue::PropertyType::String)
            {
                UCavrnusFunctionLibrary::PostStringPropertyUpdate(
                    SpaceConnection,
                    InstanceId,
                    ExposeProp.PropertyName,
                    CavrnusValue.StringValue
                );
            }
        }
        else if (FTextProperty* TextProp = CastField<FTextProperty>(Prop))
        {
            // Text properties are sent as strings
            if (CavrnusValue.PropType == Cavrnus::FPropertyValue::PropertyType::String)
            {
                UCavrnusFunctionLibrary::PostStringPropertyUpdate(
                    SpaceConnection,
                    InstanceId,
                    ExposeProp.PropertyName,
                    CavrnusValue.StringValue
                );
            }
        }
        else if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
        {
            UScriptStruct* Struct = StructProp->Struct;
            if (Struct)
            {
                if (Struct == TBaseStructure<FVector>::Get() || Struct == TBaseStructure<FVector4>::Get())
                {
                    if (CavrnusValue.PropType == Cavrnus::FPropertyValue::PropertyType::Vector)
                    {
                        UCavrnusFunctionLibrary::PostVectorPropertyUpdate(
                            SpaceConnection,
                            InstanceId,
                            ExposeProp.PropertyName,
                            CavrnusValue.VectorValue
                        );
                    }
                }
                else if (Struct == TBaseStructure<FTransform>::Get())
                {
                    if (CavrnusValue.PropType == Cavrnus::FPropertyValue::PropertyType::Transform)
                    {
                        UCavrnusFunctionLibrary::PostTransformPropertyUpdate(
                            SpaceConnection,
                            InstanceId,
                            ExposeProp.PropertyName,
                            CavrnusValue.TransformValue,
                            PostOptions
                        );
                    }
                }
                else if (Struct == TBaseStructure<FLinearColor>::Get() || Struct == TBaseStructure<FColor>::Get())
                {
                    if (CavrnusValue.PropType == Cavrnus::FPropertyValue::PropertyType::Color)
                    {
                        UCavrnusFunctionLibrary::PostColorPropertyUpdate(
                            SpaceConnection,
                            InstanceId,
                            ExposeProp.PropertyName,
                            CavrnusValue.ColorValue
                        );
                    }
                }
                else
                {
                    // For arbitrary structs, send as string
                    if (CavrnusValue.PropType == Cavrnus::FPropertyValue::PropertyType::String)
                    {
                        UCavrnusFunctionLibrary::PostStringPropertyUpdate(
                            SpaceConnection,
                            InstanceId,
                            ExposeProp.PropertyName,
                            CavrnusValue.StringValue
                        );
                    }
                }
            }
        }
    }

    // Register with manager
    Manager->RegisterSpawnedActor(SpawnedObject, SpawnedActor);

    // Add to SpawnedObjects map to prevent duplicates when journal response arrives
    Cavrnus::SpacePropertyModel* PropertyModel = 
        Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePropertyModel(SpaceConnection);
    if (PropertyModel)
    {
        SpawnedObject.SpawnedActorInstance = SpawnedActor;
        PropertyModel->SpawnedObjects.Add(InstanceId, SpawnedObject);
    }

    // Return the spawned actor immediately
    return SpawnedActor;
}

AActor* USpawnedObjectsManager::CavrnusSpawnActorByIdWithArray(
    FCavrnusSpaceConnection SpaceConnection,
    const FString& WellKnownObjectId,
    const FTransform& SpawnTransform,
    ESpawnActorCollisionHandlingMethod CollisionHandlingOverride,
    AActor* Owner,
    UCavrnusSpawnableRegistryDataAsset* DataAsset,
    const TArray<FCavrnusSpawnPropertyValue>& ExposeOnSpawnValuesArray)
{
    // Handle default values (UFUNCTIONS don't support default parameters)
    FTransform FinalTransform = SpawnTransform;
    ESpawnActorCollisionHandlingMethod FinalCollisionHandling = 
        (CollisionHandlingOverride != ESpawnActorCollisionHandlingMethod::Undefined) 
        ? CollisionHandlingOverride 
        : ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // Convert array to map
    TMap<FString, FCavrnusSpawnPropertyValue> ExposeOnSpawnValues;
    for (const FCavrnusSpawnPropertyValue& Value : ExposeOnSpawnValuesArray)
    {
        if (!Value.PropertyName.IsEmpty())
        {
            // CavrnusSpawnActorByIdWithArray: Added ExposeOnSpawn value for property
            ExposeOnSpawnValues.Add(Value.PropertyName, Value);
        }
    }

    // Call the main function with the map
    return CavrnusSpawnActorById(
        SpaceConnection,
        WellKnownObjectId,
        FinalTransform,
        FinalCollisionHandling,
        Owner,
        DataAsset,
        ExposeOnSpawnValues
    );
}

UObject* USpawnedObjectsManager::CavrnusConstructObjectFromClass(
    FCavrnusSpaceConnection SpaceConnection,
    TSubclassOf<UObject> ObjectClass,
    UObject* Outer,
    UCavrnusSpawnableRegistryDataAsset* DataAsset,
    const TMap<FString, FCavrnusSpawnPropertyValue>& ExposeOnSpawnValues)
{
    // Log all ExposeOnSpawn values from the TMap
   
    for (const auto& ValuePair : ExposeOnSpawnValues)
    {
        const FString& PropertyName = ValuePair.Key;
        const FCavrnusSpawnPropertyValue& Value = ValuePair.Value;
        
        FString ValueStr;
        switch (Value.PropertyType)
        {
        case ECavrnusSpawnPropertyType::Bool:
            ValueStr = Value.BoolValue ? TEXT("true") : TEXT("false");
            break;
        case ECavrnusSpawnPropertyType::Float:
            ValueStr = FString::SanitizeFloat(Value.FloatValue);
            break;
        case ECavrnusSpawnPropertyType::String:
            ValueStr = FString::Printf(TEXT("\"%s\""), *Value.StringValue);
            break;
        case ECavrnusSpawnPropertyType::Vector:
            ValueStr = Value.VectorValue.ToString();
            break;
        case ECavrnusSpawnPropertyType::Transform:
            ValueStr = Value.TransformValue.ToString();
            break;
        case ECavrnusSpawnPropertyType::Color:
            ValueStr = Value.ColorValue.ToString();
            break;
        case ECavrnusSpawnPropertyType::Struct:
            ValueStr = FString::Printf(TEXT("Struct(\"%s\")"), *Value.StringValue);
            break;
        default:
            ValueStr = TEXT("Unknown");
            break;
        }
        
        UE_LOG(LogCavrnusConnector, Log, TEXT("  - Property: '%s', Type: %d, Value: %s"), 
            *PropertyName, static_cast<int32>(Value.PropertyType), *ValueStr);
    }

    if (!ObjectClass)
    {
        UE_LOG(LogCavrnusConnector, Error, TEXT("CavrnusConstructObjectFromClass: ObjectClass is null"));
        return nullptr;
    }

    // Validate space connection
    if (!UCavrnusFunctionLibrary::IsConnectedToAnySpace())
    {
        UE_LOG(LogCavrnusConnector, Error, TEXT("CavrnusConstructObjectFromClass: Not connected to any space"));
        return nullptr;
    }

    // Get manager instance
    UCavrnusSubsystem* Subsystem = UCavrnusSubsystem::Get();
    if (!Subsystem || !Subsystem->RuntimeContext)
    {
        UE_LOG(LogCavrnusConnector, Error, TEXT("CavrnusConstructObjectFromClass: Subsystem is invalid"));
        return nullptr;
    }

    USpawnedObjectsManager* Manager = Subsystem->RuntimeContext->Get<USpawnedObjectsManager>();
    if (!Manager)
    {
        UE_LOG(LogCavrnusConnector, Error, TEXT("CavrnusConstructObjectFromClass: SpawnedObjectsManager is invalid"));
        return nullptr;
    }

    // Find WellKnownObjectId for this ObjectClass (required for journal operations)
    FString WellKnownObjectId;
    if (!Manager->FindWellKnownObjectIdForObjectClass(ObjectClass, DataAsset, WellKnownObjectId))
    {
        UE_LOG(LogCavrnusConnector, Warning, TEXT("CavrnusConstructObjectFromClass: ObjectClass not found in DataAsset, proceeding without WellKnownObjectId"));
        // Continue anyway - we can still construct the object, but journal operations may fail
    }

    // Create instance ID
    FString InstanceId = Cavrnus::CavrnusProtoTranslation::CreateTransientId();

    // Create object in journal and register locally
    CreateAndRegisterObject(SpaceConnection, WellKnownObjectId, InstanceId);

    // Create the object immediately (synchronous, like Unreal's ConstructObjectFromClass)
    UObject* ConstructedObject = NewObject<UObject>(Outer ? Outer : GetTransientPackage(), ObjectClass);
    if (!ConstructedObject)
    {
        UE_LOG(LogCavrnusConnector, Error, TEXT("CavrnusConstructObjectFromClass: Failed to construct object"));
        return nullptr;
    }

    // Create FCavrnusSpawnedObject for journal
    FCavrnusSpawnedObject SpawnedObject;
    SpawnedObject.SpaceConnection = SpaceConnection;
    SpawnedObject.PropertiesContainerName = InstanceId;

    // Get ExposeOnSpawn properties for validation
    TArray<FCavrnusExposeOnSpawnProperty> ExposeOnSpawnProps = 
        FCavrnusSpawnPropertyHelpers::GetExposeOnSpawnProperties(ObjectClass);
    
    // First, process all values from the map (pin values) and apply them to the constructed object
    for (const auto& PinValuePair : ExposeOnSpawnValues)
    {
        const FString& PropertyName = PinValuePair.Key;
        const FCavrnusSpawnPropertyValue& PinValue = PinValuePair.Value;
        
        // Find the corresponding property
        FProperty* Prop = nullptr;
        for (const FCavrnusExposeOnSpawnProperty& ExposeProp : ExposeOnSpawnProps)
        {
            if (ExposeProp.PropertyName == PropertyName && ExposeProp.Property)
            {
                Prop = ExposeProp.Property;
                break;
            }
        }
        
        if (!Prop)
        {
            UE_LOG(LogCavrnusConnector, Warning, TEXT("CavrnusConstructObjectFromClass: Property '%s' from pin not found in object class"), 
                *PropertyName);
            continue;
        }

        // Apply the value to the constructed object
        void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(ConstructedObject);
        if (ValuePtr)
        {
            bool bApplied = PinValue.ApplyToProperty(Prop, ValuePtr);
            if (!bApplied)
            {
                UE_LOG(LogCavrnusConnector, Warning, TEXT("CavrnusConstructObjectFromClass: Failed to apply pin value to property '%s'"), 
                    *PropertyName);
            }
        }
        else
        {
            UE_LOG(LogCavrnusConnector, Warning, TEXT("CavrnusConstructObjectFromClass: Could not get value pointer for property '%s'"), 
                *PropertyName);
        }
    }
    
    // Now post all ExposeOnSpawn properties to the journal (use pin values if available, otherwise CDO)
    for (const FCavrnusExposeOnSpawnProperty& ExposeProp : ExposeOnSpawnProps)
    {
        if (!ExposeProp.Property)
        {
            continue;
        }

        // Check if we have a value from the pin (ExposeOnSpawnValues map)
        // If not, fall back to reading from the constructed object (which now has pin values applied)
        Cavrnus::FPropertyValue CavrnusValue;

        if (const FCavrnusSpawnPropertyValue* PinValue = ExposeOnSpawnValues.Find(ExposeProp.PropertyName))
        {
            // Use value from pin
            // CavrnusConstructObjectFromClass: Posting pin value for property
            CavrnusValue = PinValue->ToCavrnusPropertyValue();
        }
        else
        {
            // Fall back to reading from constructed object (CDO values or previously applied pin values)
            // CavrnusConstructObjectFromClass: No pin value for property - reading from object"),       
            void* ValuePtr = ExposeProp.Property->ContainerPtrToValuePtr<void>(ConstructedObject);
            if (!ValuePtr)
            {
                continue;
            }

            // Convert to Cavrnus value
            CavrnusValue = 
                FCavrnusSpawnPropertyHelpers::PropertyToCavrnusValue(ExposeProp.Property, ValuePtr);
        }

        // Post to journal based on property type
        FProperty* Prop = ExposeProp.Property;
        if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop))
        {
            if (CavrnusValue.PropType == Cavrnus::FPropertyValue::PropertyType::Bool)
            {
                UCavrnusFunctionLibrary::PostBoolPropertyUpdate(
                    SpaceConnection,
                    InstanceId,
                    ExposeProp.PropertyName,
                    CavrnusValue.BoolValue
                );
            }
        }
        else if (FNumericProperty* NumericProp = CastField<FNumericProperty>(Prop))
        {
            if (CavrnusValue.PropType == Cavrnus::FPropertyValue::PropertyType::Float)
            {
                UCavrnusFunctionLibrary::PostFloatPropertyUpdate(
                    SpaceConnection,
                    InstanceId,
                    ExposeProp.PropertyName,
                    CavrnusValue.FloatValue
                );
            }
        }
        else if (FStrProperty* StrProp = CastField<FStrProperty>(Prop))
        {
            if (CavrnusValue.PropType == Cavrnus::FPropertyValue::PropertyType::String)
            {
                UCavrnusFunctionLibrary::PostStringPropertyUpdate(
                    SpaceConnection,
                    InstanceId,
                    ExposeProp.PropertyName,
                    CavrnusValue.StringValue
                );
            }
        }
        else if (FTextProperty* TextProp = CastField<FTextProperty>(Prop))
        {
            // Text properties are sent as strings
            if (CavrnusValue.PropType == Cavrnus::FPropertyValue::PropertyType::String)
            {
                UCavrnusFunctionLibrary::PostStringPropertyUpdate(
                    SpaceConnection,
                    InstanceId,
                    ExposeProp.PropertyName,
                    CavrnusValue.StringValue
                );
            }
        }
        else if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
        {
            UScriptStruct* Struct = StructProp->Struct;
            if (Struct)
            {
                if (Struct == TBaseStructure<FVector>::Get() || Struct == TBaseStructure<FVector4>::Get())
                {
                    if (CavrnusValue.PropType == Cavrnus::FPropertyValue::PropertyType::Vector)
                    {
                        UCavrnusFunctionLibrary::PostVectorPropertyUpdate(
                            SpaceConnection,
                            InstanceId,
                            ExposeProp.PropertyName,
                            CavrnusValue.VectorValue
                        );
                    }
                }
                else if (Struct == TBaseStructure<FTransform>::Get())
                {
                    if (CavrnusValue.PropType == Cavrnus::FPropertyValue::PropertyType::Transform)
                    {
                        FPropertyPostOptions PostOptions;
                        PostOptions.Smoothed = false;
                        UCavrnusFunctionLibrary::PostTransformPropertyUpdate(
                            SpaceConnection,
                            InstanceId,
                            ExposeProp.PropertyName,
                            CavrnusValue.TransformValue,
                            PostOptions
                        );
                    }
                }
                else if (Struct == TBaseStructure<FLinearColor>::Get() || Struct == TBaseStructure<FColor>::Get())
                {
                    if (CavrnusValue.PropType == Cavrnus::FPropertyValue::PropertyType::Color)
                    {
                        UCavrnusFunctionLibrary::PostColorPropertyUpdate(
                            SpaceConnection,
                            InstanceId,
                            ExposeProp.PropertyName,
                            CavrnusValue.ColorValue
                        );
                    }
                }
                else
                {
                    // For arbitrary structs, send as string
                    if (CavrnusValue.PropType == Cavrnus::FPropertyValue::PropertyType::String)
                    {
                        UCavrnusFunctionLibrary::PostStringPropertyUpdate(
                            SpaceConnection,
                            InstanceId,
                            ExposeProp.PropertyName,
                            CavrnusValue.StringValue
                        );
                    }
                }
            }
        }
    }

    // Register with manager (if tracking is needed)
    // Note: For now, we may not track UObjects the same way as Actors
    // This can be extended later if needed

    // Add to SpawnedObjects map to prevent duplicates when journal response arrives
    Cavrnus::SpacePropertyModel* PropertyModel = 
        Cavrnus::CavrnusRelayModel::GetDataModel()->GetSpacePropertyModel(SpaceConnection);
    if (PropertyModel)
    {
        // Note: FCavrnusSpawnedObject only has SpawnedActorInstance field, not SpawnedObjectInstance
        // For UObjects, we can't store them in the same way. This is a limitation that could be extended later.
        PropertyModel->SpawnedObjects.Add(InstanceId, SpawnedObject);
    }

    // Return the constructed object immediately
    return ConstructedObject;
}

UObject* USpawnedObjectsManager::CavrnusConstructObjectFromClassWithArray(
    FCavrnusSpaceConnection SpaceConnection,
    TSubclassOf<UObject> ObjectClass,
    UObject* Outer,
    UCavrnusSpawnableRegistryDataAsset* DataAsset,
    const TArray<FCavrnusSpawnPropertyValue>& ExposeOnSpawnValuesArray)
{
    // Convert array to map
    TMap<FString, FCavrnusSpawnPropertyValue> ExposeOnSpawnValues;
    for (const FCavrnusSpawnPropertyValue& Value : ExposeOnSpawnValuesArray)
    {
        if (!Value.PropertyName.IsEmpty())
        {
            // CavrnusConstructObjectFromClassWithArray: Added ExposeOnSpawn value for property
            ExposeOnSpawnValues.Add(Value.PropertyName, Value);
        }
    }

    // CavrnusConstructObjectFromClassWithArray: Passing ExposeOnSpawn values to CavrnusConstructObjectFromClass

    // Call the main function with the map
    return CavrnusConstructObjectFromClass(
        SpaceConnection,
        ObjectClass,
        Outer,
        DataAsset,
        ExposeOnSpawnValues
    );
}

void USpawnedObjectsManager::RegisterCustomSpawnObjectClass(const FString& WellKnownObjectId, TSubclassOf<UCavrnusPendingSpawnObject> PendingSpawnClass)
{
    if (WellKnownObjectId.IsEmpty() || !PendingSpawnClass)
    {
        UE_LOG(LogCavrnusConnector, Warning, TEXT("RegisterCustomSpawnObjectClass: Invalid parameters"));
        return;
    }

    CustomSpawnObjectClasses.Add(WellKnownObjectId, PendingSpawnClass);
    UE_LOG(LogCavrnusConnector, Log, TEXT("Registered custom spawn object class %s for WellKnownObjectId %s"), 
        *PendingSpawnClass->GetName(), *WellKnownObjectId);
}

void USpawnedObjectsManager::UnregisterCustomSpawnObjectClass(const FString& WellKnownObjectId)
{
    if (CustomSpawnObjectClasses.Remove(WellKnownObjectId) > 0)
    {
        UE_LOG(LogCavrnusConnector, Log, TEXT("Unregistered custom spawn object class for WellKnownObjectId %s"), *WellKnownObjectId);
    }
}

void USpawnedObjectsManager::RegisterSpawnableDataAsset(UCavrnusSpawnableRegistryDataAsset* DataAsset)
{
    if (!DataAsset)
    {
        UE_LOG(LogCavrnusConnector, Warning, TEXT("RegisterSpawnableDataAsset: DataAsset is null"));
        return;
    }

    // Check if already registered
    for (const TWeakObjectPtr<UCavrnusSpawnableRegistryDataAsset>& Registered : RegisteredSpawnableDataAssets)
    {
        if (Registered.Get() == DataAsset)
        {
            UE_LOG(LogCavrnusConnector, Verbose, TEXT("RegisterSpawnableDataAsset: DataAsset %s already registered"), *DataAsset->GetName());
            return;
        }
    }

    RegisteredSpawnableDataAssets.Add(DataAsset);
    UE_LOG(LogCavrnusConnector, Log, TEXT("Registered spawnable DataAsset: %s"), *DataAsset->GetName());
}

void USpawnedObjectsManager::UnregisterSpawnableDataAsset(UCavrnusSpawnableRegistryDataAsset* DataAsset)
{
    if (!DataAsset)
    {
        return;
    }

    int32 Removed = RegisteredSpawnableDataAssets.RemoveAll([DataAsset](const TWeakObjectPtr<UCavrnusSpawnableRegistryDataAsset>& Registered)
    {
        return Registered.Get() == DataAsset;
    });

    if (Removed > 0)
    {
        UE_LOG(LogCavrnusConnector, Log, TEXT("Unregistered spawnable DataAsset: %s"), *DataAsset->GetName());
    }
}

void USpawnedObjectsManager::RegisterSpawnedActor(const FCavrnusSpawnedObject& SpawnedObject, AActor* SpawnedActor)
{
    if (!SpawnedActor)
    {
        return;
    }

    const FPropertiesContainer Key(SpawnedObject.PropertiesContainerName);
    spawnedActors.Add(Key, SpawnedActor);

    // Remove from pending spawns if it exists
    PendingSpawns.Remove(Key);
}

bool USpawnedObjectsManager::FindSpawnableClassOrMesh(
    const FString& WellKnownObjectId,
    UCavrnusSpawnableRegistryDataAsset* OptionalDataAsset,
    TSubclassOf<AActor>& OutActorClass,
    UStaticMesh*& OutStaticMesh)
{
    OutActorClass = nullptr;
    OutStaticMesh = nullptr;

    const FName KeyName(*WellKnownObjectId);

    // Helper lambda to check a single DataAsset
    auto CheckDataAsset = [&](UCavrnusSpawnableRegistryDataAsset* DataAsset) -> bool
    {
        if (!DataAsset)
        {
            return false;
        }

        // Only look for actor class - matching Unreal Engine's SpawnActor behavior
        // SpawnActor only accepts AActor classes, not asset instances
        TOptional<TSoftClassPtr<AActor>> ActorClassOpt = DataAsset->GetActorClassForKey(KeyName);
        if (ActorClassOpt.IsSet())
        {
            TSoftClassPtr<AActor> SoftClass = ActorClassOpt.GetValue();
            UClass* LoadedClass = SoftClass.LoadSynchronous();
            if (LoadedClass && LoadedClass->IsChildOf(AActor::StaticClass()))
            {
                OutActorClass = LoadedClass;
                return true;
            }
        }

        return false;
    };

    // Step 1: Check optional DataAsset if provided (highest priority)
    if (OptionalDataAsset)
    {
        if (CheckDataAsset(OptionalDataAsset))
        {
            return true;
        }
    }

    // Step 2: Check default DataAsset from DataAssetManager
    UCavrnusSpawnableRegistryDataAsset* DefaultDataAsset = nullptr;
    UCavrnusSubsystem* Subsystem = UCavrnusSubsystem::Get();
    if (Subsystem && Subsystem->RuntimeContext)
    {
        UCavrnusDataAssetManager* DataAssetManager = Subsystem->RuntimeContext->Get<UCavrnusDataAssetManager>();
        if (DataAssetManager)
        {
            DefaultDataAsset = DataAssetManager->GetAsset<UCavrnusSpawnableRegistryDataAsset>();
        }
    }

    if (DefaultDataAsset && CheckDataAsset(DefaultDataAsset))
    {
        return true;
    }

    // Step 3: Check all registered DataAssets (modules like CVT)
    for (const TWeakObjectPtr<UCavrnusSpawnableRegistryDataAsset>& RegisteredDataAsset : RegisteredSpawnableDataAssets)
    {
        if (UCavrnusSpawnableRegistryDataAsset* DataAsset = RegisteredDataAsset.Get())
        {
            // Skip if we already checked this one (shouldn't happen, but safety check)
            if (DataAsset == OptionalDataAsset || DataAsset == DefaultDataAsset)
            {
                continue;
            }

            if (CheckDataAsset(DataAsset))
            {
                return true;
            }
        }
    }

    return false;
}

bool USpawnedObjectsManager::FindWellKnownObjectIdForActorClass(
    TSubclassOf<AActor> ActorClass,
    UCavrnusSpawnableRegistryDataAsset* OptionalDataAsset,
    FString& OutWellKnownObjectId)
{
    if (!ActorClass)
    {
        return false;
    }

    OutWellKnownObjectId.Empty();

    // Helper lambda to check a single DataAsset
    auto CheckDataAsset = [&](UCavrnusSpawnableRegistryDataAsset* DataAssetToCheck) -> bool
    {
        if (!DataAssetToCheck)
        {
            return false;
        }

        // Iterate through all entries to find one that matches the actor class
        const TArray<FCavrnusSpawnableEntry>& Entries = DataAssetToCheck->GetEntries();
        for (const FCavrnusSpawnableEntry& Entry : Entries)
        {
            // Check if this entry has an actor class
            if (!Entry.ActorClass.IsNull())
            {
                // Load the soft class pointer
                UClass* LoadedClass = Entry.ActorClass.LoadSynchronous();
                if (LoadedClass && LoadedClass == ActorClass)
                {
                    // Found a match! Return the key
                    OutWellKnownObjectId = Entry.Key.ToString();
                    return true;
                }
            }
        }

        return false;
    };

    // Step 1: Check optional DataAsset if provided (highest priority)
    if (OptionalDataAsset)
    {
        if (CheckDataAsset(OptionalDataAsset))
        {
            return true;
        }
    }

    // Step 2: Check default DataAsset from DataAssetManager
    UCavrnusSpawnableRegistryDataAsset* DefaultDataAsset = nullptr;
    UCavrnusSubsystem* Subsystem = UCavrnusSubsystem::Get();
    if (Subsystem && Subsystem->RuntimeContext)
    {
        UCavrnusDataAssetManager* DataAssetManager = Subsystem->RuntimeContext->Get<UCavrnusDataAssetManager>();
        if (DataAssetManager)
        {
            DefaultDataAsset = DataAssetManager->GetAsset<UCavrnusSpawnableRegistryDataAsset>();
        }
    }

    if (DefaultDataAsset && CheckDataAsset(DefaultDataAsset))
    {
        return true;
    }

    // Step 3: Check registered DataAssets (modules like CVT)
    for (const TWeakObjectPtr<UCavrnusSpawnableRegistryDataAsset>& WeakDataAsset : RegisteredSpawnableDataAssets)
    {
        if (UCavrnusSpawnableRegistryDataAsset* RegisteredDataAsset = WeakDataAsset.Get())
        {
            // Skip if we already checked this one (shouldn't happen, but safety check)
            if (RegisteredDataAsset == OptionalDataAsset || RegisteredDataAsset == DefaultDataAsset)
            {
                continue;
            }

            if (CheckDataAsset(RegisteredDataAsset))
            {
                return true;
            }
        }
    }

    return false;
}

bool USpawnedObjectsManager::FindSpawnableObjectClass(
    const FString& WellKnownObjectId,
    UCavrnusSpawnableRegistryDataAsset* OptionalDataAsset,
    TSubclassOf<UObject>& OutObjectClass)
{
    OutObjectClass = nullptr;

    if (WellKnownObjectId.IsEmpty())
    {
        return false;
    }

    const FName KeyName(*WellKnownObjectId);

    // Helper lambda to check a single DataAsset
    auto CheckDataAsset = [&](UCavrnusSpawnableRegistryDataAsset* DataAsset) -> bool
    {
        if (!DataAsset)
        {
            return false;
        }

        // Try to find object class
        TOptional<TSoftClassPtr<UObject>> ObjectClassOpt = DataAsset->GetObjectClassForKey(KeyName);
        if (ObjectClassOpt.IsSet())
        {
            TSoftClassPtr<UObject> SoftClass = ObjectClassOpt.GetValue();
            UClass* LoadedClass = SoftClass.LoadSynchronous();
            if (LoadedClass && LoadedClass->IsChildOf(UObject::StaticClass()))
            {
                OutObjectClass = LoadedClass;
                return true;
            }
        }

        return false;
    };

    // Step 1: Check optional DataAsset if provided (highest priority)
    if (OptionalDataAsset)
    {
        if (CheckDataAsset(OptionalDataAsset))
        {
            return true;
        }
    }

    // Step 2: Check default DataAsset from DataAssetManager
    UCavrnusSpawnableRegistryDataAsset* DefaultDataAsset = nullptr;
    UCavrnusSubsystem* Subsystem = UCavrnusSubsystem::Get();
    if (Subsystem && Subsystem->RuntimeContext)
    {
        UCavrnusDataAssetManager* DataAssetManager = Subsystem->RuntimeContext->Get<UCavrnusDataAssetManager>();
        if (DataAssetManager)
        {
            DefaultDataAsset = DataAssetManager->GetAsset<UCavrnusSpawnableRegistryDataAsset>();
        }
    }

    if (DefaultDataAsset && CheckDataAsset(DefaultDataAsset))
    {
        return true;
    }

    // Step 3: Check registered DataAssets (modules like CVT)
    for (const TWeakObjectPtr<UCavrnusSpawnableRegistryDataAsset>& WeakDataAsset : RegisteredSpawnableDataAssets)
    {
        if (UCavrnusSpawnableRegistryDataAsset* RegisteredDataAsset = WeakDataAsset.Get())
        {
            // Skip if we already checked this one
            if (RegisteredDataAsset == OptionalDataAsset || RegisteredDataAsset == DefaultDataAsset)
            {
                continue;
            }

            if (CheckDataAsset(RegisteredDataAsset))
            {
                return true;
            }
        }
    }

    return false;
}

bool USpawnedObjectsManager::FindWellKnownObjectIdForObjectClass(
    TSubclassOf<UObject> ObjectClass,
    UCavrnusSpawnableRegistryDataAsset* OptionalDataAsset,
    FString& OutWellKnownObjectId)
{
    if (!ObjectClass)
    {
        return false;
    }

    OutWellKnownObjectId.Empty();

    // Helper lambda to check a single DataAsset
    auto CheckDataAsset = [&](UCavrnusSpawnableRegistryDataAsset* DataAssetToCheck) -> bool
    {
        if (!DataAssetToCheck)
        {
            return false;
        }

        // Iterate through all entries to find one that matches the object class
        const TArray<FCavrnusSpawnableEntry>& Entries = DataAssetToCheck->GetEntries();
        for (const FCavrnusSpawnableEntry& Entry : Entries)
        {
            // Check if this entry has an object class (and not an actor class)
            if (!Entry.ObjectClass.IsNull() && Entry.ActorClass.IsNull())
            {
                // Load the soft class pointer
                UClass* LoadedClass = Entry.ObjectClass.LoadSynchronous();
                if (LoadedClass && LoadedClass == ObjectClass)
                {
                    // Found a match! Return the key
                    OutWellKnownObjectId = Entry.Key.ToString();
                    return true;
                }
            }
        }

        return false;
    };

    // Step 1: Check optional DataAsset if provided (highest priority)
    if (OptionalDataAsset)
    {
        if (CheckDataAsset(OptionalDataAsset))
        {
            return true;
        }
    }

    // Step 2: Check default DataAsset from DataAssetManager
    UCavrnusSpawnableRegistryDataAsset* DefaultDataAsset = nullptr;
    UCavrnusSubsystem* Subsystem = UCavrnusSubsystem::Get();
    if (Subsystem && Subsystem->RuntimeContext)
    {
        UCavrnusDataAssetManager* DataAssetManager = Subsystem->RuntimeContext->Get<UCavrnusDataAssetManager>();
        if (DataAssetManager)
        {
            DefaultDataAsset = DataAssetManager->GetAsset<UCavrnusSpawnableRegistryDataAsset>();
        }
    }

    if (DefaultDataAsset && CheckDataAsset(DefaultDataAsset))
    {
        return true;
    }

    // Step 3: Check registered DataAssets (modules like CVT)
    for (const TWeakObjectPtr<UCavrnusSpawnableRegistryDataAsset>& WeakDataAsset : RegisteredSpawnableDataAssets)
    {
        if (UCavrnusSpawnableRegistryDataAsset* RegisteredDataAsset = WeakDataAsset.Get())
        {
            // Skip if we already checked this one
            if (RegisteredDataAsset == OptionalDataAsset || RegisteredDataAsset == DefaultDataAsset)
            {
                continue;
            }

            if (CheckDataAsset(RegisteredDataAsset))
            {
                return true;
            }
        }
    }

    return false;
}

UCavrnusPendingSpawnObject* USpawnedObjectsManager::CreatePendingConstructObject(
    const FCavrnusSpawnedObject& SpawnedObject,
    const FString& WellKnownObjectId,
    TSubclassOf<UObject> ObjectClass)
{
    // Check factory registry for custom class
    TSubclassOf<UCavrnusPendingSpawnObject> PendingSpawnClass = nullptr;
    if (!WellKnownObjectId.IsEmpty() && CustomSpawnObjectClasses.Contains(WellKnownObjectId))
    {
        PendingSpawnClass = CustomSpawnObjectClasses[WellKnownObjectId];
    }

    // Use default class if no custom class registered
    if (!PendingSpawnClass)
    {
        PendingSpawnClass = UCavrnusPendingSpawnObject::StaticClass();
    }

    // Create the pending spawn object
    UCavrnusPendingSpawnObject* PendingSpawn = NewObject<UCavrnusPendingSpawnObject>(GetTransientPackage(), PendingSpawnClass);
    if (PendingSpawn)
    {
        PendingSpawn->SetupWithObjectClass(SpawnedObject, WellKnownObjectId, ObjectClass, this);
        PendingSpawns.Add(SpawnedObject.PropertiesContainerName, PendingSpawn);
    }

    return PendingSpawn;
}

UCavrnusPendingSpawnObject* USpawnedObjectsManager::CreatePendingSpawnObjectWithActorClass(
    const FCavrnusSpawnedObject& SpawnedObject,
    const FString& WellKnownObjectId,
    TSubclassOf<AActor> ActorClass,
    ESpawnActorCollisionHandlingMethod CollisionHandling)
{
    // Check factory registry for custom class
    TSubclassOf<UCavrnusPendingSpawnObject> PendingSpawnClass = nullptr;
    if (!WellKnownObjectId.IsEmpty() && CustomSpawnObjectClasses.Contains(WellKnownObjectId))
    {
        PendingSpawnClass = CustomSpawnObjectClasses[WellKnownObjectId];
    }

    // Use custom class if found, otherwise use base class
    if (!PendingSpawnClass)
    {
        PendingSpawnClass = UCavrnusPendingSpawnObject::StaticClass();
    }

    // Create the pending spawn object
    UCavrnusPendingSpawnObject* PendingSpawn = NewObject<UCavrnusPendingSpawnObject>(GetTransientPackage(), PendingSpawnClass);
    if (!PendingSpawn)
    {
        UE_LOG(LogCavrnusConnector, Error, TEXT("CreatePendingSpawnObjectWithActorClass: Failed to create pending spawn object"));
        return nullptr;
    }

    // Setup the pending spawn
    PendingSpawn->SetupWithActorClass(SpawnedObject, WellKnownObjectId, ActorClass, CollisionHandling, this);

    // Track it
    const FPropertiesContainer Key(SpawnedObject.PropertiesContainerName);
    PendingSpawns.Add(Key, PendingSpawn);

    return PendingSpawn;
}

void USpawnedObjectsManager::CreateAndRegisterObject(
    const FCavrnusSpaceConnection& SpaceConnection,
    const FString& WellKnownObjectId,
    const FString& InstanceId)
{
    // Send createObject command to journal (matches old SpawnObject pattern)
    Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(
        Cavrnus::CavrnusProtoTranslation::BuildCreateOp(SpaceConnection, WellKnownObjectId, InstanceId)
    );
    
    // Register locally (matches old SpawnObject pattern)
    // HandleSpaceObjectAdded will check if it exists and return early if already registered
    Cavrnus::CavrnusRelayModel::GetDataModel()->HandleSpaceObjectAdded(
        Cavrnus::CavrnusProtoTranslation::BuildObjectAdded(SpaceConnection, WellKnownObjectId, InstanceId)
    );
}

