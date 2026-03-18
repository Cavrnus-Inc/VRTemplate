// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Helpers/CavrnusPropertyReflectionHelpers.h"
#include "UObject/UnrealType.h"
#include "UObject/Class.h"
#include "UObject/TextProperty.h"

// ─── Internal helpers ───────────────────────────────────────────────────────

const void* FCavrnusPropertyReflectionHelpers::GetValuePtr(const FProperty* Prop, const UObject* Object)
{
	if (!Prop || !Object) return nullptr;
	return Prop->ContainerPtrToValuePtr<void>(Object);
}

TArray<FCavrnusStringEnumOption> FCavrnusPropertyReflectionHelpers::ExtractEnumOptions(const FProperty* Prop)
{
	TArray<FCavrnusStringEnumOption> Options;

	UEnum* Enum = nullptr;

	if (const FEnumProperty* EnumProp = CastField<FEnumProperty>(Prop))
	{
		Enum = EnumProp->GetEnum();
	}
	else if (const FByteProperty* ByteProp = CastField<FByteProperty>(Prop))
	{
		Enum = ByteProp->Enum;
	}

	if (!Enum) return Options;

	// NumEnums() - 1 to skip the auto-generated _MAX entry
	const int32 Count = Enum->NumEnums() - 1;
	Options.Reserve(Count);

	for (int32 i = 0; i < Count; ++i)
	{
#if WITH_EDITORONLY_DATA
		if (Enum->HasMetaData(TEXT("Hidden"), i))
			continue;
#endif

		FCavrnusStringEnumOption Opt;
		Opt.EnumValue = Enum->GetNameStringByIndex(i);
		Opt.DisplayText = Enum->GetDisplayNameTextByIndex(i).ToString();
		Options.Add(Opt);
	}

	return Options;
}

// ─── ExtractMetadata ────────────────────────────────────────────────────────

FCavrnusPropertyMetadata FCavrnusPropertyReflectionHelpers::ExtractMetadata(const FProperty* Prop)
{
	FCavrnusPropertyMetadata Meta;
	if (!Prop) return Meta;

	Meta.bReadOnly = Prop->HasAnyPropertyFlags(CPF_BlueprintReadOnly);

#if WITH_EDITORONLY_DATA
	Meta.DisplayName = Prop->GetDisplayNameText().ToString();
	Meta.Description = Prop->GetToolTipText().ToString();

	if (Prop->HasMetaData(TEXT("AdvancedDisplay")))
	{
		Meta.bAdvanced = true;
	}

	if (Prop->HasMetaData(TEXT("Category")))
	{
		Meta.Category = Prop->GetMetaData(TEXT("Category"));
	}
#endif

	return Meta;
}

// ─── TryPopulateFloat ───────────────────────────────────────────────────────

bool FCavrnusPropertyReflectionHelpers::TryPopulateFloat(
	const FProperty* Prop, const UObject* Object,
	FCavrnusFloatPropertyDefinition& OutDef)
{
	if (!Prop || !Object) return false;

	const void* ValuePtr = GetValuePtr(Prop, Object);
	if (!ValuePtr) return false;

	float CurrentValue = 0.0f;

	if (const FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop))
	{
		CurrentValue = FloatProp->GetPropertyValue(ValuePtr);
	}
	else if (const FDoubleProperty* DoubleProp = CastField<FDoubleProperty>(Prop))
	{
		CurrentValue = static_cast<float>(DoubleProp->GetPropertyValue(ValuePtr));
	}
	else if (const FIntProperty* IntProp = CastField<FIntProperty>(Prop))
	{
		CurrentValue = static_cast<float>(IntProp->GetPropertyValue(ValuePtr));
	}
	else if (const FInt64Property* Int64Prop = CastField<FInt64Property>(Prop))
	{
		CurrentValue = static_cast<float>(Int64Prop->GetPropertyValue(ValuePtr));
	}
	else if (const FNumericProperty* NumericProp = CastField<FNumericProperty>(Prop))
	{
		if (NumericProp->IsInteger())
			CurrentValue = static_cast<float>(NumericProp->GetSignedIntPropertyValue(ValuePtr));
		else
			CurrentValue = NumericProp->GetFloatingPointPropertyValue(ValuePtr);
	}
	else
	{
		return false;
	}

	OutDef.Metadata = ExtractMetadata(Prop);
	OutDef.DefaultValue = CurrentValue;

#if WITH_EDITORONLY_DATA
	bool bHasUIMin = Prop->HasMetaData(TEXT("UIMin"));
	bool bHasUIMax = Prop->HasMetaData(TEXT("UIMax"));
	if (bHasUIMin || bHasUIMax)
	{
		OutDef.bHasRange = true;
		if (bHasUIMin)
			OutDef.UiRangeMin = FCString::Atof(*Prop->GetMetaData(TEXT("UIMin")));
		if (bHasUIMax)
			OutDef.UiRangeMax = FCString::Atof(*Prop->GetMetaData(TEXT("UIMax")));
	}

	// Fallback to ClampMin/ClampMax if no UIMin/UIMax
	if (!OutDef.bHasRange)
	{
		bool bHasClampMin = Prop->HasMetaData(TEXT("ClampMin"));
		bool bHasClampMax = Prop->HasMetaData(TEXT("ClampMax"));
		if (bHasClampMin || bHasClampMax)
		{
			OutDef.bHasRange = true;
			if (bHasClampMin)
				OutDef.UiRangeMin = FCString::Atof(*Prop->GetMetaData(TEXT("ClampMin")));
			if (bHasClampMax)
				OutDef.UiRangeMax = FCString::Atof(*Prop->GetMetaData(TEXT("ClampMax")));
		}
	}

	if (Prop->HasMetaData(TEXT("Delta")))
	{
		OutDef.UiIncrement = FCString::Atof(*Prop->GetMetaData(TEXT("Delta")));
	}
#endif

	return true;
}

// ─── TryPopulateString ──────────────────────────────────────────────────────

bool FCavrnusPropertyReflectionHelpers::TryPopulateString(
	const FProperty* Prop, const UObject* Object,
	FCavrnusStringPropertyDefinition& OutDef)
{
	if (!Prop || !Object) return false;

	const void* ValuePtr = GetValuePtr(Prop, Object);
	if (!ValuePtr) return false;

	FString CurrentValue;

	if (const FStrProperty* StrProp = CastField<FStrProperty>(Prop))
	{
		CurrentValue = StrProp->GetPropertyValue(ValuePtr);
	}
	else if (const FTextProperty* TextProp = CastField<FTextProperty>(Prop))
	{
		CurrentValue = TextProp->GetPropertyValue(ValuePtr).ToString();
	}
	else if (const FNameProperty* NameProp = CastField<FNameProperty>(Prop))
	{
		CurrentValue = NameProp->GetPropertyValue(ValuePtr).ToString();
	}
	else if (const FEnumProperty* EnumProp = CastField<FEnumProperty>(Prop))
	{
		const FNumericProperty* UnderlyingProp = EnumProp->GetUnderlyingProperty();
		int64 EnumValue = UnderlyingProp->GetSignedIntPropertyValue(
			EnumProp->ContainerPtrToValuePtr<void>(Object));
		UEnum* Enum = EnumProp->GetEnum();
		if (Enum)
		{
			CurrentValue = Enum->GetNameStringByValue(EnumValue);
			OutDef.EnumOptions = ExtractEnumOptions(Prop);
		}
	}
	else if (const FByteProperty* ByteProp = CastField<FByteProperty>(Prop))
	{
		if (ByteProp->Enum)
		{
			uint8 ByteValue = ByteProp->GetPropertyValue(ValuePtr);
			CurrentValue = ByteProp->Enum->GetNameStringByValue(ByteValue);
			OutDef.EnumOptions = ExtractEnumOptions(Prop);
		}
		else
		{
			return false; // Plain byte — let TryPopulateFloat handle it
		}
	}
	else
	{
		return false;
	}

	OutDef.Metadata = ExtractMetadata(Prop);
	OutDef.DefaultValue = CurrentValue;

#if WITH_EDITORONLY_DATA
	if (Prop->HasMetaData(TEXT("MultiLine")))
	{
		OutDef.bIsMultiLine = true;
	}
#endif

	return true;
}

// ─── TryPopulateBool ────────────────────────────────────────────────────────

bool FCavrnusPropertyReflectionHelpers::TryPopulateBool(
	const FProperty* Prop, const UObject* Object,
	FCavrnusBoolPropertyDefinition& OutDef)
{
	const FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop);
	if (!BoolProp) return false;

	const void* ValuePtr = GetValuePtr(Prop, Object);
	if (!ValuePtr) return false;

	OutDef.Metadata = ExtractMetadata(Prop);
	OutDef.DefaultValue = BoolProp->GetPropertyValue(ValuePtr);
	return true;
}

// ─── TryPopulateColor ───────────────────────────────────────────────────────

bool FCavrnusPropertyReflectionHelpers::TryPopulateColor(
	const FProperty* Prop, const UObject* Object,
	FCavrnusColorPropertyDefinition& OutDef)
{
	const FStructProperty* StructProp = CastField<FStructProperty>(Prop);
	if (!StructProp) return false;

	const void* ValuePtr = GetValuePtr(Prop, Object);
	if (!ValuePtr) return false;

	if (StructProp->Struct == TBaseStructure<FLinearColor>::Get())
	{
		OutDef.DefaultValue = *static_cast<const FLinearColor*>(ValuePtr);
	}
	else if (StructProp->Struct == TBaseStructure<FColor>::Get())
	{
		OutDef.DefaultValue = FLinearColor(*static_cast<const FColor*>(ValuePtr));
	}
	else
	{
		return false;
	}

	OutDef.Metadata = ExtractMetadata(Prop);

#if WITH_EDITORONLY_DATA
	if (Prop->HasMetaData(TEXT("HDR")))
	{
		OutDef.bAllowHdr = true;
	}
	// UE uses HideAlphaChannel to hide alpha; Cavrnus uses bUsesAlpha to show it
	OutDef.bUsesAlpha = !Prop->HasMetaData(TEXT("HideAlphaChannel"));
#else
	OutDef.bUsesAlpha = true;
#endif

	return true;
}

// ─── TryPopulateVector ──────────────────────────────────────────────────────

bool FCavrnusPropertyReflectionHelpers::TryPopulateVector(
	const FProperty* Prop, const UObject* Object,
	FCavrnusVectorPropertyDefinition& OutDef)
{
	const FStructProperty* StructProp = CastField<FStructProperty>(Prop);
	if (!StructProp) return false;

	const void* ValuePtr = GetValuePtr(Prop, Object);
	if (!ValuePtr) return false;

	UScriptStruct* Struct = StructProp->Struct;

	if (Struct == TBaseStructure<FVector>::Get())
	{
		const FVector* Vec = static_cast<const FVector*>(ValuePtr);
		OutDef.DefaultValue = FVector4(Vec->X, Vec->Y, Vec->Z, 0.0);
		OutDef.VectorUsage = ECavrnusVectorUsage::Point;
	}
	else if (Struct == TBaseStructure<FVector4>::Get())
	{
		OutDef.DefaultValue = *static_cast<const FVector4*>(ValuePtr);
		OutDef.VectorUsage = ECavrnusVectorUsage::Point;
	}
	else if (Struct == TBaseStructure<FVector2D>::Get())
	{
		const FVector2D* Vec2 = static_cast<const FVector2D*>(ValuePtr);
		OutDef.DefaultValue = FVector4(Vec2->X, Vec2->Y, 0.0, 0.0);
		OutDef.VectorUsage = ECavrnusVectorUsage::Point2D;
	}
	else if (Struct == TBaseStructure<FRotator>::Get())
	{
		const FRotator* Rot = static_cast<const FRotator*>(ValuePtr);
		OutDef.DefaultValue = FVector4(Rot->Roll, Rot->Pitch, Rot->Yaw, 0.0);
		OutDef.VectorUsage = ECavrnusVectorUsage::Eulers;
	}
	else
	{
		return false;
	}

	OutDef.Metadata = ExtractMetadata(Prop);
	return true;
}

// ─── TryPopulateTransform ───────────────────────────────────────────────────

bool FCavrnusPropertyReflectionHelpers::TryPopulateTransform(
	const FProperty* Prop, const UObject* Object,
	FCavrnusTransformPropertyDefinition& OutDef)
{
	const FStructProperty* StructProp = CastField<FStructProperty>(Prop);
	if (!StructProp) return false;
	if (StructProp->Struct != TBaseStructure<FTransform>::Get())
		return false;

	const void* ValuePtr = GetValuePtr(Prop, Object);
	if (!ValuePtr) return false;

	OutDef.Metadata = ExtractMetadata(Prop);
	OutDef.DefaultValue = *static_cast<const FTransform*>(ValuePtr);
	return true;
}

// ─── AutoPopulate ───────────────────────────────────────────────────────────

FCavrnusPropertyReflectionHelpers::FPopulateResult
FCavrnusPropertyReflectionHelpers::AutoPopulate(
	const FProperty* Prop, const UObject* Object)
{
	FPopulateResult Result;
	if (!Prop || !Object) return Result;

	// Dispatch order:
	// Bool → String (catches enums) → Float (remaining numerics)
	// → Color → Transform → Vector

	if (TryPopulateBool(Prop, Object, Result.BoolDef))
	{
		Result.Type = EPopulatedType::Bool;
	}
	else if (TryPopulateString(Prop, Object, Result.StringDef))
	{
		Result.Type = EPopulatedType::String;
	}
	else if (TryPopulateFloat(Prop, Object, Result.FloatDef))
	{
		Result.Type = EPopulatedType::Float;
	}
	else if (TryPopulateColor(Prop, Object, Result.ColorDef))
	{
		Result.Type = EPopulatedType::Color;
	}
	else if (TryPopulateTransform(Prop, Object, Result.TransformDef))
	{
		Result.Type = EPopulatedType::Transform;
	}
	else if (TryPopulateVector(Prop, Object, Result.VectorDef))
	{
		Result.Type = EPopulatedType::Vector;
	}

	return Result;
}
