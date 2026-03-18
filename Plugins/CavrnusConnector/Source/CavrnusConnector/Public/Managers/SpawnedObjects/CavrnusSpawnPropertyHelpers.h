// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/UnrealType.h"
#include "Types/CavrnusPropertyValue.h"
#include "CavrnusSpawnPropertyHelpers.generated.h"

/**
 * @brief Structure to hold information about an ExposeOnSpawn property.
 */
USTRUCT()
struct FCavrnusExposeOnSpawnProperty
{
    GENERATED_BODY()

    /** The name of the property */
    UPROPERTY()
    FString PropertyName;

    /** The FProperty pointer (not serialized, used at runtime) */
    FProperty* Property = nullptr;

    /** Whether this is a struct property */
    UPROPERTY()
    bool bIsStruct = false;

    /** The struct type if it's a struct property */
    UPROPERTY()
    UScriptStruct* StructType = nullptr;
};

/**
 * @brief Helper functions for working with ExposeOnSpawn properties and property conversion.
 */
class CAVRNUSCONNECTOR_API FCavrnusSpawnPropertyHelpers
{
public:
    /**
     * @brief Gets all properties with ExposeOnSpawn metadata from an actor class.
     * 
     * @param ActorClass The actor class to search for ExposeOnSpawn properties
     * @return Array of ExposeOnSpawn properties found in the class hierarchy
     */
    static TArray<FCavrnusExposeOnSpawnProperty> GetExposeOnSpawnProperties(UClass* ActorClass);

    /**
     * @brief Converts an FProperty value to a Cavrnus FPropertyValue.
     * 
     * @param Prop The property to read from
     * @param ValuePtr Pointer to the property value in memory
     * @return Cavrnus property value representation
     */
    static Cavrnus::FPropertyValue PropertyToCavrnusValue(FProperty* Prop, void* ValuePtr);

    /**
     * @brief Converts a Cavrnus FPropertyValue back to an FProperty value.
     * 
     * @param Value The Cavrnus property value
     * @param Prop The property to write to
     * @param ValuePtr Pointer to the property value in memory
     */
    static void CavrnusValueToProperty(const Cavrnus::FPropertyValue& Value, FProperty* Prop, void* ValuePtr);

private:
    /**
     * @brief Checks if a property has ExposeOnSpawn flag/metadata.
     */
    static bool HasExposeOnSpawn(FProperty* Prop);

    /**
     * @brief Checks if a property type is a reference type (should be filtered out).
     */
    static bool IsReferenceType(FProperty* Prop);
};

