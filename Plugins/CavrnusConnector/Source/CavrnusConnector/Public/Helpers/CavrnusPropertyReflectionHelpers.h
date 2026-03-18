// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/UnrealType.h"
#include "Types/CavrnusPropertyDefinition.h"

/**
 * @brief Extracts UE FProperty reflection metadata and populates
 *        Cavrnus property definition structs.
 *
 * Used by the auto-sharing system to introspect UObject properties
 * and produce the same definition structs that manual
 * Define*PropertyDefinition calls accept.
 *
 * DisplayName, Description, bReadOnly, and DefaultValue are available
 * in all builds. Category, bAdvanced, ranges, HDR, MultiLine, etc.
 * require WITH_EDITORONLY_DATA.
 */
class CAVRNUSCONNECTOR_API FCavrnusPropertyReflectionHelpers
{
public:
	// ── Core: populate base metadata from any FProperty ──────────

	/**
	 * Fills FCavrnusPropertyMetadata from FProperty reflection data.
	 */
	static FCavrnusPropertyMetadata ExtractMetadata(const FProperty* Prop);

	// ── Per-type populators ─────────────────────────────────────
	// Each returns true if the FProperty was a compatible type,
	// false if the FProperty type doesn't match.

	static bool TryPopulateFloat(const FProperty* Prop, const UObject* Object, FCavrnusFloatPropertyDefinition& OutDef);
	static bool TryPopulateString(const FProperty* Prop, const UObject* Object, FCavrnusStringPropertyDefinition& OutDef);
	static bool TryPopulateBool(const FProperty* Prop, const UObject* Object, FCavrnusBoolPropertyDefinition& OutDef);
	static bool TryPopulateColor(const FProperty* Prop, const UObject* Object, FCavrnusColorPropertyDefinition& OutDef);
	static bool TryPopulateVector(const FProperty* Prop, const UObject* Object, FCavrnusVectorPropertyDefinition& OutDef);
	static bool TryPopulateTransform(const FProperty* Prop, const UObject* Object, FCavrnusTransformPropertyDefinition& OutDef);

	// ── Convenience: auto-dispatch to correct type ──────────────

	enum class EPopulatedType : uint8
	{
		None,
		Float,
		String,
		Bool,
		Color,
		Vector,
		Transform
	};

	struct FPopulateResult
	{
		EPopulatedType Type = EPopulatedType::None;

		FCavrnusFloatPropertyDefinition FloatDef;
		FCavrnusStringPropertyDefinition StringDef;
		FCavrnusBoolPropertyDefinition BoolDef;
		FCavrnusColorPropertyDefinition ColorDef;
		FCavrnusVectorPropertyDefinition VectorDef;
		FCavrnusTransformPropertyDefinition TransformDef;
	};

	/**
	 * Inspects the FProperty type, dispatches to the correct
	 * TryPopulate* method, and returns the result.
	 */
	static FPopulateResult AutoPopulate(const FProperty* Prop, const UObject* Object);

private:
	static const void* GetValuePtr(const FProperty* Prop, const UObject* Object);
	static TArray<FCavrnusStringEnumOption> ExtractEnumOptions(const FProperty* Prop);
};
