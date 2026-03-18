// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Managers/SpawnedObjects/CavrnusSpawnPropertyHelpers.h"
#include "UObject/UnrealType.h"
#include "UObject/Class.h"
#include "UObject/TextProperty.h"
#include "Runtime/Launch/Resources/Version.h"

TArray<FCavrnusExposeOnSpawnProperty> FCavrnusSpawnPropertyHelpers::GetExposeOnSpawnProperties(UClass* ActorClass)
{
    TArray<FCavrnusExposeOnSpawnProperty> Result;

    if (!ActorClass)
    {
        return Result;
    }

    // Iterate through all properties in the class hierarchy (includes inherited properties)
    for (TFieldIterator<FProperty> PropIt(ActorClass, EFieldIterationFlags::None); PropIt; ++PropIt)
    {
        FProperty* Prop = *PropIt;
        if (!Prop)
        {
            continue;
        }

        // Check if property has ExposeOnSpawn
        if (!HasExposeOnSpawn(Prop))
        {
            continue;
        }

        // Filter out reference types
        if (IsReferenceType(Prop))
        {
            continue;
        }

        // Create entry
        FCavrnusExposeOnSpawnProperty Entry;
        Entry.PropertyName = Prop->GetName();
        Entry.Property = Prop;

        // Check if it's a struct property
        if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
        {
            Entry.bIsStruct = true;
            Entry.StructType = StructProp->Struct;
        }

        Result.Add(Entry);
    }

    return Result;
}

bool FCavrnusSpawnPropertyHelpers::HasExposeOnSpawn(FProperty* Prop)
{
    if (!Prop)
    {
        return false;
    }

    // Check property flags (standard Unreal method)
    if (Prop->HasAnyPropertyFlags(CPF_ExposeOnSpawn))
    {
        return true;
    }

// Note : No longer metadata check here.  Unclear if that's going to b a problem.

    return false;
}
bool FCavrnusSpawnPropertyHelpers::IsReferenceType(FProperty* Prop)
{
    if (!Prop)
    {
        return false;
    }

    // Check for object reference properties
    if (FObjectPropertyBase* ObjectProp = CastField<FObjectPropertyBase>(Prop))
    {
        return true;
    }

    // Check for interface properties
    if (FInterfaceProperty* InterfaceProp = CastField<FInterfaceProperty>(Prop))
    {
        return true;
    }

    // Check for delegate properties
    if (FDelegateProperty* DelegateProp = CastField<FDelegateProperty>(Prop))
    {
        return true;
    }

    if (FMulticastDelegateProperty* MulticastDelegateProp = CastField<FMulticastDelegateProperty>(Prop))
    {
        return true;
    }

    return false;
}

Cavrnus::FPropertyValue FCavrnusSpawnPropertyHelpers::PropertyToCavrnusValue(FProperty* Prop, void* ValuePtr)
{
    if (!Prop || !ValuePtr)
    {
        return Cavrnus::FPropertyValue();
    }

    // Check for native Cavrnus types first
    if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop))
    {
        return Cavrnus::FPropertyValue::BoolPropValue(BoolProp->GetPropertyValue(ValuePtr));
    }

    if (FIntProperty* IntProp = CastField<FIntProperty>(Prop))
    {
        return Cavrnus::FPropertyValue::FloatPropValue(static_cast<float>(IntProp->GetPropertyValue(ValuePtr)));
    }

    if (FInt64Property* Int64Prop = CastField<FInt64Property>(Prop))
    {
        return Cavrnus::FPropertyValue::FloatPropValue(static_cast<float>(Int64Prop->GetPropertyValue(ValuePtr)));
    }

    if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop))
    {
        return Cavrnus::FPropertyValue::FloatPropValue(FloatProp->GetPropertyValue(ValuePtr));
    }

    if (FDoubleProperty* DoubleProp = CastField<FDoubleProperty>(Prop))
    {
        return Cavrnus::FPropertyValue::FloatPropValue(static_cast<float>(DoubleProp->GetPropertyValue(ValuePtr)));
    }

    if (FStrProperty* StrProp = CastField<FStrProperty>(Prop))
    {
        return Cavrnus::FPropertyValue::StringPropValue(StrProp->GetPropertyValue(ValuePtr));
    }

    if (FTextProperty* TextProp = CastField<FTextProperty>(Prop))
    {
        return Cavrnus::FPropertyValue::StringPropValue(TextProp->GetPropertyValue(ValuePtr).ToString());
    }

    // Check for vector types
    if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
    {
        UScriptStruct* Struct = StructProp->Struct;
        if (!Struct)
        {
            return Cavrnus::FPropertyValue();
        }

        // Check for native struct types
        if (Struct == TBaseStructure<FVector>::Get())
        {
            FVector* VecPtr = static_cast<FVector*>(ValuePtr);
            return Cavrnus::FPropertyValue::VectorPropValue(FVector4(VecPtr->X, VecPtr->Y, VecPtr->Z, 0.0f));
        }

        if (Struct == TBaseStructure<FVector4>::Get())
        {
            return Cavrnus::FPropertyValue::VectorPropValue(*static_cast<FVector4*>(ValuePtr));
        }

        if (Struct == TBaseStructure<FRotator>::Get())
        {
            FRotator* RotPtr = static_cast<FRotator*>(ValuePtr);
            // Convert rotator to vector for storage (or we could store as string)
            // For now, convert to string
            FString TextValue;
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 0
            StructProp->ExportTextItem(TextValue, ValuePtr, nullptr, nullptr, PPF_None, nullptr);
#else
            StructProp->ExportText_InContainer(0, TextValue, ValuePtr, nullptr, nullptr, PPF_None, nullptr);
#endif
            return Cavrnus::FPropertyValue::StringPropValue(TextValue);
        }

        if (Struct == TBaseStructure<FTransform>::Get())
        {
            return Cavrnus::FPropertyValue::TransformPropValue(*static_cast<FTransform*>(ValuePtr));
        }

        if (Struct == TBaseStructure<FLinearColor>::Get())
        {
            return Cavrnus::FPropertyValue::ColorPropValue(*static_cast<FLinearColor*>(ValuePtr));
        }

        if (Struct == TBaseStructure<FColor>::Get())
        {
            FColor* ColorPtr = static_cast<FColor*>(ValuePtr);
            FLinearColor LinearColor(*ColorPtr);
            return Cavrnus::FPropertyValue::ColorPropValue(LinearColor);
        }

        // For arbitrary structs, convert to string using ExportText
        FString TextValue;
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 0
        StructProp->ExportTextItem(TextValue, ValuePtr, nullptr, nullptr, PPF_None, nullptr);
#else
        StructProp->ExportText_InContainer(0, TextValue, ValuePtr, nullptr, nullptr, PPF_None, nullptr);
#endif
        return Cavrnus::FPropertyValue::StringPropValue(TextValue);
    }

    // For numeric properties that weren't caught above
    if (FNumericProperty* NumericProp = CastField<FNumericProperty>(Prop))
    {
        if (NumericProp->IsInteger())
        {
            return Cavrnus::FPropertyValue::FloatPropValue(static_cast<float>(NumericProp->GetSignedIntPropertyValue(ValuePtr)));
        }
        else
        {
            return Cavrnus::FPropertyValue::FloatPropValue(NumericProp->GetFloatingPointPropertyValue(ValuePtr));
        }
    }

    return Cavrnus::FPropertyValue();
}

void FCavrnusSpawnPropertyHelpers::CavrnusValueToProperty(const Cavrnus::FPropertyValue& Value, FProperty* Prop, void* ValuePtr)
{
    if (!Prop || !ValuePtr)
    {
        return;
    }

    // Handle native Cavrnus types
    if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop))
    {
        if (Value.PropType == Cavrnus::FPropertyValue::PropertyType::Bool)
        {
            BoolProp->SetPropertyValue(ValuePtr, Value.BoolValue);
        }
        return;
    }

    if (FIntProperty* IntProp = CastField<FIntProperty>(Prop))
    {
        if (Value.PropType == Cavrnus::FPropertyValue::PropertyType::Float)
        {
            IntProp->SetPropertyValue(ValuePtr, static_cast<int32>(Value.FloatValue));
        }
        return;
    }

    if (FInt64Property* Int64Prop = CastField<FInt64Property>(Prop))
    {
        if (Value.PropType == Cavrnus::FPropertyValue::PropertyType::Float)
        {
            Int64Prop->SetPropertyValue(ValuePtr, static_cast<int64>(Value.FloatValue));
        }
        return;
    }

    if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop))
    {
        if (Value.PropType == Cavrnus::FPropertyValue::PropertyType::Float)
        {
            FloatProp->SetPropertyValue(ValuePtr, Value.FloatValue);
        }
        return;
    }

    if (FDoubleProperty* DoubleProp = CastField<FDoubleProperty>(Prop))
    {
        if (Value.PropType == Cavrnus::FPropertyValue::PropertyType::Float)
        {
            DoubleProp->SetPropertyValue(ValuePtr, static_cast<double>(Value.FloatValue));
        }
        return;
    }

    if (FStrProperty* StrProp = CastField<FStrProperty>(Prop))
    {
        if (Value.PropType == Cavrnus::FPropertyValue::PropertyType::String)
        {
            StrProp->SetPropertyValue(ValuePtr, Value.StringValue);
        }
        return;
    }

    if (FTextProperty* TextProp = CastField<FTextProperty>(Prop))
    {
        if (Value.PropType == Cavrnus::FPropertyValue::PropertyType::String)
        {
            TextProp->SetPropertyValue(ValuePtr, FText::FromString(Value.StringValue));
        }
        return;
    }

    // Handle struct properties
    if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
    {
        UScriptStruct* Struct = StructProp->Struct;
        if (!Struct)
        {
            return;
        }

        // Handle native struct types
        if (Struct == TBaseStructure<FVector>::Get())
        {
            if (Value.PropType == Cavrnus::FPropertyValue::PropertyType::Vector)
            {
                FVector* VecPtr = static_cast<FVector*>(ValuePtr);
                VecPtr->X = Value.VectorValue.X;
                VecPtr->Y = Value.VectorValue.Y;
                VecPtr->Z = Value.VectorValue.Z;
            }
            return;
        }

        if (Struct == TBaseStructure<FVector4>::Get())
        {
            if (Value.PropType == Cavrnus::FPropertyValue::PropertyType::Vector)
            {
                *static_cast<FVector4*>(ValuePtr) = Value.VectorValue;
            }
            return;
        }

        if (Struct == TBaseStructure<FRotator>::Get())
        {
            if (Value.PropType == Cavrnus::FPropertyValue::PropertyType::String)
            {
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 0
                StructProp->ImportText(*Value.StringValue, ValuePtr, PPF_None, nullptr);
#elif ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
                StructProp->ImportText_InContainer(*Value.StringValue, ValuePtr, nullptr, PPF_None);
#endif
            }
            return;
        }

        if (Struct == TBaseStructure<FTransform>::Get())
        {
            if (Value.PropType == Cavrnus::FPropertyValue::PropertyType::Transform)
            {
                *static_cast<FTransform*>(ValuePtr) = Value.TransformValue;
            }
            return;
        }

        if (Struct == TBaseStructure<FLinearColor>::Get())
        {
            if (Value.PropType == Cavrnus::FPropertyValue::PropertyType::Color)
            {
                *static_cast<FLinearColor*>(ValuePtr) = Value.ColorValue;
            }
            return;
        }

        if (Struct == TBaseStructure<FColor>::Get())
        {
            if (Value.PropType == Cavrnus::FPropertyValue::PropertyType::Color)
            {
                FColor* ColorPtr = static_cast<FColor*>(ValuePtr);
                *ColorPtr = Value.ColorValue.ToFColor(true);
            }
            return;
        }

        // For arbitrary structs, use ImportText
        if (Value.PropType == Cavrnus::FPropertyValue::PropertyType::String && !Value.StringValue.IsEmpty())
        {
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 0
            if (!StructProp->ImportText(*Value.StringValue, ValuePtr, PPF_None, nullptr))
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("[CavrnusValueToProperty] Failed to import struct text for '%s'"),
                    *Struct->GetName());
            }
#elif ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
            if (!StructProp->ImportText_InContainer(*Value.StringValue, ValuePtr, nullptr, PPF_None))
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("[CavrnusValueToProperty] Failed to import struct text for '%s'"),
                    *Struct->GetName());
            }
#endif
        }
        return;
    }

    // For numeric properties that weren't caught above
    if (FNumericProperty* NumericProp = CastField<FNumericProperty>(Prop))
    {
        if (Value.PropType == Cavrnus::FPropertyValue::PropertyType::Float)
        {
            if (NumericProp->IsInteger())
            {
                NumericProp->SetIntPropertyValue(ValuePtr, static_cast<int64>(Value.FloatValue));
            }
            else
            {
                NumericProp->SetFloatingPointPropertyValue(ValuePtr, Value.FloatValue);
            }
        }
        return;
    }
}

