// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/UnrealType.h"
#include "Types/CavrnusPropertyValue.h"
#include "CavrnusSpawnPropertyValue.generated.h"

/**
 * @brief Enumeration of supported property types for ExposeOnSpawn values.
 */
UENUM()
enum class ECavrnusSpawnPropertyType : uint8
{
    Bool,
    Float,
    String,
    Vector,
    Transform,
    Color,
    Struct  // Arbitrary structs stored as serialized strings
};

/**
 * @brief Structure to hold ExposeOnSpawn property values passed from Blueprint pins.
 * 
 * This struct can hold values for all supported property types, including structs
 * which are serialized to strings using FProperty ExportText/ImportText functions.
 */
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusSpawnPropertyValue
{
    GENERATED_BODY()

    /** The name of the property */
    UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Spawn")
    FString PropertyName;

    /** The type of the property value */
    UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Spawn")
    ECavrnusSpawnPropertyType PropertyType = ECavrnusSpawnPropertyType::Bool;

    /** Bool value (used when PropertyType == Bool) */
    UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Spawn")
    bool BoolValue = false;

    /** Float/Int value (used when PropertyType == Float) */
    UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Spawn")
    float FloatValue = 0.0f;

    /** String value (used when PropertyType == String or Struct) */
    UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Spawn")
    FString StringValue;

    /** Vector value (used when PropertyType == Vector) */
    UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Spawn")
    FVector4 VectorValue = FVector4();

    /** Transform value (used when PropertyType == Transform) */
    UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Spawn")
    FTransform TransformValue;

    /** Color value (used when PropertyType == Color) */
    UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Spawn")
    FLinearColor ColorValue = FLinearColor::White;

    /** The struct type for struct properties (used when PropertyType == Struct) */
    UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Spawn")
    TSoftObjectPtr<UScriptStruct> StructType;

    /**
     * @brief Default constructor
     */
    FCavrnusSpawnPropertyValue()
        : PropertyName(TEXT(""))
        , PropertyType(ECavrnusSpawnPropertyType::Bool)
        , BoolValue(false)
        , FloatValue(0.0f)
        , StringValue(TEXT(""))
        , VectorValue(FVector4())
        , TransformValue(FTransform())
        , ColorValue(FLinearColor::White)
    {
    }

    /**
     * @brief Converts this value to a Cavrnus::FPropertyValue for journal posting.
     * 
     * @return Cavrnus property value representation
     */
    Cavrnus::FPropertyValue ToCavrnusPropertyValue() const;

    /**
     * @brief Creates a FCavrnusSpawnPropertyValue from a Cavrnus::FPropertyValue.
     * 
     * @param Value The Cavrnus property value
     * @param PropertyName The name of the property
     * @param StructType Optional struct type if this is a struct property
     * @return FCavrnusSpawnPropertyValue instance
     */
    static FCavrnusSpawnPropertyValue FromCavrnusPropertyValue(
        const Cavrnus::FPropertyValue& Value,
        const FString& PropertyName,
        UScriptStruct* StructType = nullptr
    );

    /**
     * @brief Creates a FCavrnusSpawnPropertyValue from an FProperty value.
     * 
     * @param Prop The property to read from
     * @param ValuePtr Pointer to the property value in memory
     * @param PropertyName The name of the property
     * @return FCavrnusSpawnPropertyValue instance
     */
    static FCavrnusSpawnPropertyValue FromPropertyValue(
        FProperty* Prop,
        void* ValuePtr,
        const FString& PropertyName
    );

    /**
     * @brief Applies this value to an FProperty.
     * 
     * @param Prop The property to write to
     * @param ValuePtr Pointer to the property value in memory
     * @return True if the value was successfully applied
     */
    bool ApplyToProperty(FProperty* Prop, void* ValuePtr) const;
};

