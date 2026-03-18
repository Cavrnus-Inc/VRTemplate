// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusPropertyDefinition.h
 * @brief Blueprint-visible types for defining property metadata alongside default values.
 *
 * These types mirror the protobuf metadata structures (Property::PropertyMetadata,
 * Property::ScalarEditingMetadata, etc.) as UE-friendly USTRUCTs and UENUMs.
 */

#pragma once

#include "CoreMinimal.h"
#include "CavrnusPropertyDefinition.generated.h"

// ============================================
// Enums
// ============================================

/**
 * @brief Interpretation hint for scalar (float) properties.
 * Maps to Property::ScalarEditingMetadata::ScalarInterpretationEnum.
 */
UENUM(BlueprintType)
enum class ECavrnusScalarType : uint8
{
	Standard	UMETA(DisplayName = "Standard"),
	Time		UMETA(DisplayName = "Time"),
	Playback	UMETA(DisplayName = "Playback"),
	Scaler		UMETA(DisplayName = "Scaler")
};

/**
 * @brief Interpretation hint for vector properties.
 * Maps to Property::VectorEditingMetadata::VectorInterpretationEnum.
 */
UENUM(BlueprintType)
enum class ECavrnusVectorUsage : uint8
{
	Point		UMETA(DisplayName = "Point"),
	Direction	UMETA(DisplayName = "Direction"),
	Eulers		UMETA(DisplayName = "Eulers"),
	Scale		UMETA(DisplayName = "Scale"),
	Quaternion	UMETA(DisplayName = "Quaternion"),
	Point2D		UMETA(DisplayName = "Point2D"),
	Direction2D	UMETA(DisplayName = "Direction2D"),
	Scale2D		UMETA(DisplayName = "Scale2D"),
	OffsetScale	UMETA(DisplayName = "OffsetScale")
};

// ============================================
// Base metadata struct
// ============================================

/**
 * @brief Base metadata common to all property types.
 * Maps to Property::PropertyMetadata in the protobuf schema.
 */
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusPropertyMetadata
{
	GENERATED_BODY()

	/** Display name shown in UI (maps to PropertyMetadata.name). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	FString DisplayName;

	/** Human-readable description of the property. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	FString Description;

	/** If true the property cannot be edited by users. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	bool bReadOnly = false;

	/** Grouping category for UI organization. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	FString Category;

	/** Sort order within the category. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	float Order = 0.0f;

	/** If true the property is hidden by default and shown only in advanced views. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	bool bAdvanced = false;
};

// ============================================
// String enum option
// ============================================

/**
 * @brief A single option in a string enumeration.
 * Maps to Property::StringValueEnumerationOption.
 */
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusStringEnumOption
{
	GENERATED_BODY()

	/** The raw value stored in the property. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	FString EnumValue;

	/** The human-readable text shown in UI. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	FString DisplayText;
};

// ============================================
// Per-type property definitions
// ============================================

/**
 * @brief Definition for a string property including metadata, default value, and editing hints.
 */
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusStringPropertyDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	FCavrnusPropertyMetadata Metadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	FString DefaultValue;

	/** Available options when the string acts as an enumeration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	TArray<FCavrnusStringEnumOption> EnumOptions;

	/** If true the editor shows a multi-line text box. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	bool bIsMultiLine = false;

	/** If true the content is treated as script / code. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	bool bIsScript = false;
};

/**
 * @brief Definition for a float property including metadata, default value, and editing hints.
 */
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusFloatPropertyDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	FCavrnusPropertyMetadata Metadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	float DefaultValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	ECavrnusScalarType ScalarType = ECavrnusScalarType::Standard;

	/** Minimum value shown in UI sliders. Only used when bHasRange is true. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	float UiRangeMin = 0.0f;

	/** Maximum value shown in UI sliders. Only used when bHasRange is true. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	float UiRangeMax = 1.0f;

	/** Increment step for UI sliders / spinboxes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	float UiIncrement = 0.0f;

	/** When true the UiRangeMin / UiRangeMax values are sent as editing metadata. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	bool bHasRange = false;
};

/**
 * @brief Definition for a color property including metadata, default value, and editing hints.
 */
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusColorPropertyDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	FCavrnusPropertyMetadata Metadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	FLinearColor DefaultValue = FLinearColor::White;

	/** Allow HDR (high dynamic range) values. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	bool bAllowHdr = false;

	/** Whether the alpha channel is editable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	bool bUsesAlpha = false;
};

/**
 * @brief Definition for a vector property including metadata, default value, and editing hints.
 */
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusVectorPropertyDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	FCavrnusPropertyMetadata Metadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	FVector4 DefaultValue = FVector4(0, 0, 0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	ECavrnusVectorUsage VectorUsage = ECavrnusVectorUsage::Point;
};

/**
 * @brief Definition for a transform property including metadata, default value, and editing hints.
 */
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusTransformPropertyDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	FCavrnusPropertyMetadata Metadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	FTransform DefaultValue;

	/** Allow the user to set this transform from their own avatar transform. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	bool bAllowSetFromUserTransform = false;

	/** Allow the property to be unset / cleared. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	bool bAllowUnset = false;
};

/**
 * @brief Definition for a boolean property including metadata and default value.
 */
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusBoolPropertyDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	FCavrnusPropertyMetadata Metadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Properties")
	bool DefaultValue = false;
};
