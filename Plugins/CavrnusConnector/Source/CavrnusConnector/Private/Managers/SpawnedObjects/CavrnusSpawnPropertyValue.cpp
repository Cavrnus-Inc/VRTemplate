// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Managers/SpawnedObjects/CavrnusSpawnPropertyValue.h"
#include "UObject/UnrealType.h"
#include "UObject/Class.h"
#include "UObject/TextProperty.h" 
#include "Runtime/Launch/Resources/Version.h"

Cavrnus::FPropertyValue FCavrnusSpawnPropertyValue::ToCavrnusPropertyValue() const
{
    switch (PropertyType)
    {
    case ECavrnusSpawnPropertyType::Bool:
        return Cavrnus::FPropertyValue::BoolPropValue(BoolValue);
    
    case ECavrnusSpawnPropertyType::Float:
        return Cavrnus::FPropertyValue::FloatPropValue(FloatValue);
    
    case ECavrnusSpawnPropertyType::String:
        return Cavrnus::FPropertyValue::StringPropValue(StringValue);
    
    case ECavrnusSpawnPropertyType::Vector:
        return Cavrnus::FPropertyValue::VectorPropValue(VectorValue);
    
    case ECavrnusSpawnPropertyType::Transform:
        return Cavrnus::FPropertyValue::TransformPropValue(TransformValue);
    
    case ECavrnusSpawnPropertyType::Color:
        return Cavrnus::FPropertyValue::ColorPropValue(ColorValue);
    
    case ECavrnusSpawnPropertyType::Struct:
        // Structs are stored as strings, so return as string
        return Cavrnus::FPropertyValue::StringPropValue(StringValue);
    
    default:
        return Cavrnus::FPropertyValue();
    }
}

FCavrnusSpawnPropertyValue FCavrnusSpawnPropertyValue::FromCavrnusPropertyValue(
    const Cavrnus::FPropertyValue& Value,
    const FString& InPropertyName,
    UScriptStruct* InStructType)
{
    FCavrnusSpawnPropertyValue Result;
    Result.PropertyName = InPropertyName;
    Result.StructType = InStructType;

    switch (Value.PropType)
    {
    case Cavrnus::FPropertyValue::PropertyType::Bool:
        Result.PropertyType = ECavrnusSpawnPropertyType::Bool;
        Result.BoolValue = Value.BoolValue;
        break;
    
    case Cavrnus::FPropertyValue::PropertyType::Float:
        Result.PropertyType = ECavrnusSpawnPropertyType::Float;
        Result.FloatValue = Value.FloatValue;
        break;
    
    case Cavrnus::FPropertyValue::PropertyType::String:
        Result.PropertyType = ECavrnusSpawnPropertyType::String;
        Result.StringValue = Value.StringValue;
        break;
    
    case Cavrnus::FPropertyValue::PropertyType::Vector:
        Result.PropertyType = ECavrnusSpawnPropertyType::Vector;
        Result.VectorValue = Value.VectorValue;
        break;
    
    case Cavrnus::FPropertyValue::PropertyType::Transform:
        Result.PropertyType = ECavrnusSpawnPropertyType::Transform;
        Result.TransformValue = Value.TransformValue;
        break;
    
    case Cavrnus::FPropertyValue::PropertyType::Color:
        Result.PropertyType = ECavrnusSpawnPropertyType::Color;
        Result.ColorValue = Value.ColorValue;
        break;
    
    default:
        Result.PropertyType = ECavrnusSpawnPropertyType::String;
        Result.StringValue = TEXT("");
        break;
    }

    return Result;
}

FCavrnusSpawnPropertyValue FCavrnusSpawnPropertyValue::FromPropertyValue(
    FProperty* Prop,
    void* ValuePtr,
    const FString& InPropertyName)
{
    FCavrnusSpawnPropertyValue Result;
    Result.PropertyName = InPropertyName;

    if (!Prop || !ValuePtr)
    {
        return Result;
    }

    // Check for native types first
    if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop))
    {
        Result.PropertyType = ECavrnusSpawnPropertyType::Bool;
        Result.BoolValue = BoolProp->GetPropertyValue(ValuePtr);
        return Result;
    }

    if (FNumericProperty* NumericProp = CastField<FNumericProperty>(Prop))
    {
        Result.PropertyType = ECavrnusSpawnPropertyType::Float;
        if (NumericProp->IsInteger())
        {
            Result.FloatValue = static_cast<float>(NumericProp->GetSignedIntPropertyValue(ValuePtr));
        }
        else
        {
            Result.FloatValue = NumericProp->GetFloatingPointPropertyValue(ValuePtr);
        }
        return Result;
    }

    if (FStrProperty* StrProp = CastField<FStrProperty>(Prop))
    {
        Result.PropertyType = ECavrnusSpawnPropertyType::String;
        Result.StringValue = StrProp->GetPropertyValue(ValuePtr);
        return Result;
    }

    if (FTextProperty* TextProp = CastField<FTextProperty>(Prop))
    {
        Result.PropertyType = ECavrnusSpawnPropertyType::String;
        Result.StringValue = TextProp->GetPropertyValue(ValuePtr).ToString();
        return Result;
    }

    // Handle struct properties
    if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
    {
        UScriptStruct* Struct = StructProp->Struct;
        if (!Struct)
        {
            return Result;
        }

        Result.StructType = Struct;

        // Check for native struct types
        if (Struct == TBaseStructure<FVector>::Get())
        {
            Result.PropertyType = ECavrnusSpawnPropertyType::Vector;
            FVector* VecPtr = static_cast<FVector*>(ValuePtr);
            Result.VectorValue = FVector4(VecPtr->X, VecPtr->Y, VecPtr->Z, 0.0f);
            return Result;
        }

        if (Struct == TBaseStructure<FVector4>::Get())
        {
            Result.PropertyType = ECavrnusSpawnPropertyType::Vector;
            Result.VectorValue = *static_cast<FVector4*>(ValuePtr);
            return Result;
        }

        if (Struct == TBaseStructure<FTransform>::Get())
        {
            Result.PropertyType = ECavrnusSpawnPropertyType::Transform;
            Result.TransformValue = *static_cast<FTransform*>(ValuePtr);
            return Result;
        }

        if (Struct == TBaseStructure<FLinearColor>::Get())
        {
            Result.PropertyType = ECavrnusSpawnPropertyType::Color;
            Result.ColorValue = *static_cast<FLinearColor*>(ValuePtr);
            return Result;
        }

        if (Struct == TBaseStructure<FColor>::Get())
        {
            Result.PropertyType = ECavrnusSpawnPropertyType::Color;
            FColor* ColorPtr = static_cast<FColor*>(ValuePtr);
            Result.ColorValue = FLinearColor(*ColorPtr);
            return Result;
        }

        // For arbitrary structs, serialize to string
        Result.PropertyType = ECavrnusSpawnPropertyType::Struct;
        FString TextValue;
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 0
        StructProp->ExportTextItem(TextValue, ValuePtr, nullptr, nullptr, PPF_None, nullptr);
#else
        StructProp->ExportText_InContainer(0, TextValue, ValuePtr, nullptr, nullptr, PPF_None, nullptr);
#endif
        Result.StringValue = TextValue;
        return Result;
    }

    return Result;
}

bool FCavrnusSpawnPropertyValue::ApplyToProperty(FProperty* Prop, void* ValuePtr) const
{
    if (!Prop || !ValuePtr)
    {
        return false;
    }

    switch (PropertyType)
    {
    case ECavrnusSpawnPropertyType::Bool:
    {
        if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop))
        {
            BoolProp->SetPropertyValue(ValuePtr, BoolValue);
            return true;
        }
        break;
    }

    case ECavrnusSpawnPropertyType::Float:
    {
        if (FNumericProperty* NumericProp = CastField<FNumericProperty>(Prop))
        {
            if (NumericProp->IsInteger())
            {
                NumericProp->SetIntPropertyValue(ValuePtr, static_cast<int64>(FloatValue));
            }
            else
            {
                NumericProp->SetFloatingPointPropertyValue(ValuePtr, FloatValue);
            }
            return true;
        }
        break;
    }

    case ECavrnusSpawnPropertyType::String:
    {
        if (FStrProperty* StrProp = CastField<FStrProperty>(Prop))
        {
            StrProp->SetPropertyValue(ValuePtr, StringValue);
            return true;
        }
        if (FTextProperty* TextProp = CastField<FTextProperty>(Prop))
        {
            TextProp->SetPropertyValue(ValuePtr, FText::FromString(StringValue));
            return true;
        }
        break;
    }

    case ECavrnusSpawnPropertyType::Vector:
    {
        if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
        {
            UScriptStruct* Struct = StructProp->Struct;
            if (Struct == TBaseStructure<FVector>::Get())
            {
                FVector* VecPtr = static_cast<FVector*>(ValuePtr);
                VecPtr->X = VectorValue.X;
                VecPtr->Y = VectorValue.Y;
                VecPtr->Z = VectorValue.Z;
                return true;
            }
            if (Struct == TBaseStructure<FVector4>::Get())
            {
                *static_cast<FVector4*>(ValuePtr) = VectorValue;
                return true;
            }
        }
        break;
    }

    case ECavrnusSpawnPropertyType::Transform:
    {
        if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
        {
            UScriptStruct* Struct = StructProp->Struct;
            if (Struct == TBaseStructure<FTransform>::Get())
            {
                *static_cast<FTransform*>(ValuePtr) = TransformValue;
                return true;
            }
        }
        break;
    }

    case ECavrnusSpawnPropertyType::Color:
    {
        if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
        {
            UScriptStruct* Struct = StructProp->Struct;
            if (Struct == TBaseStructure<FLinearColor>::Get())
            {
                *static_cast<FLinearColor*>(ValuePtr) = ColorValue;
                return true;
            }
            if (Struct == TBaseStructure<FColor>::Get())
            {
                FColor* ColorPtr = static_cast<FColor*>(ValuePtr);
                *ColorPtr = ColorValue.ToFColor(true);
                return true;
            }
        }
        break;
    }

    case ECavrnusSpawnPropertyType::Struct:
    {
        if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
        {
            UScriptStruct* Struct = StructProp->Struct;
            if (!Struct || StringValue.IsEmpty())
            {
                return false;
            }

            // Deserialize struct from string using ImportText
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 0
            if (StructProp->ImportText(*StringValue, ValuePtr, PPF_None, nullptr))
            {
                return true;
            }
#elif ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
            if (StructProp->ImportText_InContainer(*StringValue, ValuePtr, nullptr, PPF_None))
            {
                return true;
            }
#endif
        }
        break;
    }

    default:
        break;
    }

    return false;
}

