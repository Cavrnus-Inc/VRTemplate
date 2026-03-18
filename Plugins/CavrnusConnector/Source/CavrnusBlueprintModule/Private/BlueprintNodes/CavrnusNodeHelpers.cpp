// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "BlueprintNodes/CavrnusNodeHelpers.h"
#include "Managers/SpawnedObjects/CavrnusSpawnPropertyHelpers.h"
#include "Managers/SpawnedObjects/CavrnusSpawnPropertyValue.h"
#include "CavrnusBlueprintModule.h"
#include "Engine/Engine.h"
#include "UObject/UnrealType.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraph/EdGraph.h"
#include "K2Node_MakeStruct.h"
#include "K2Node_MakeArray.h"
#include "KismetCompiler.h"
#include "Kismet2/CompilerResultsLog.h"
#include "Runtime/Launch/Resources/Version.h"

FEdGraphPinType FCavrnusNodeHelpers::GetPinTypeForProperty(FProperty* Prop)
{
    FEdGraphPinType PinType;
    PinType.PinCategory = NAME_None;

    if (!Prop)
    {
        return PinType;
    }

    if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop))
    {
        PinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
    }
    else if (FNumericProperty* NumericProp = CastField<FNumericProperty>(Prop))
    {
        if (NumericProp->IsInteger())
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Int;
        }
        else
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Real;
            PinType.PinSubCategory = UEdGraphSchema_K2::PC_Double;
        }
    }
    else if (FStrProperty* StrProp = CastField<FStrProperty>(Prop))
    {
        PinType.PinCategory = UEdGraphSchema_K2::PC_String;
    }
    else if (FTextProperty* TextProp = CastField<FTextProperty>(Prop))
    {
        PinType.PinCategory = UEdGraphSchema_K2::PC_String; // Text is represented as string in Blueprint
    }
    else if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
    {
        UScriptStruct* Struct = StructProp->Struct;
        if (Struct)
        {
            // First, try using Unreal's schema conversion (handles all BlueprintType structs properly)
            const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();
            FEdGraphPinType SchemaPinType;
            bool bSchemaConversionSucceeded = false;
            
            if (Schema && Schema->ConvertPropertyToPinType(Prop, SchemaPinType))
            {
                // If schema conversion succeeded and it's a struct pin, use it
                if (SchemaPinType.PinCategory == UEdGraphSchema_K2::PC_Struct && SchemaPinType.PinSubCategoryObject.IsValid())
                {
                    PinType = SchemaPinType;
                    bSchemaConversionSucceeded = true;
                }
                // If schema conversion returned something else (e.g., a special type), use it
                else if (SchemaPinType.PinCategory != NAME_None)
                {
                    PinType = SchemaPinType;
                    bSchemaConversionSucceeded = true;
                }
            }
            
            // Fallback: If schema conversion didn't work or didn't return a valid type, check hardcoded types
            if (!bSchemaConversionSucceeded)
            {
                if (Struct == TBaseStructure<FVector>::Get())
                {
                    PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
                    PinType.PinSubCategoryObject = TBaseStructure<FVector>::Get();
                }
                else if (Struct == TBaseStructure<FVector4>::Get())
                {
                    PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
                    PinType.PinSubCategoryObject = TBaseStructure<FVector4>::Get();
                }
                else if (Struct == TBaseStructure<FRotator>::Get())
                {
                    PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
                    PinType.PinSubCategoryObject = TBaseStructure<FRotator>::Get();
                }
                else if (Struct == TBaseStructure<FTransform>::Get())
                {
                    PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
                    PinType.PinSubCategoryObject = TBaseStructure<FTransform>::Get();
                }
                else if (Struct == TBaseStructure<FLinearColor>::Get())
                {
                    PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
                    PinType.PinSubCategoryObject = TBaseStructure<FLinearColor>::Get();
                }
                else if (Struct == TBaseStructure<FColor>::Get())
                {
                    PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
                    PinType.PinSubCategoryObject = TBaseStructure<FColor>::Get();
                }
                else
                {
                    // For arbitrary structs that aren't BlueprintType, use string pin (they're sent as strings)
                    PinType.PinCategory = UEdGraphSchema_K2::PC_String;
                }
            }
        }
    }

    return PinType;
}

void FCavrnusNodeHelpers::RemoveExposeOnSpawnPins(
    UK2Node* Node,
    UClass* CurrentClass,
    const TArray<FCavrnusExposeOnSpawnProperty>& CachedProperties,
    TFunction<UClass*()> GetClassFunc)
{
    if (!Node)
    {
        return;
    }

    // Remove all ExposeOnSpawn property pins
    TArray<UEdGraphPin*> PinsToRemove;
    
    // Get current ExposeOnSpawn properties to identify which pins to remove
    TArray<FCavrnusExposeOnSpawnProperty> CurrentExposeProperties;
    if (CurrentClass)
    {
        CurrentExposeProperties = FCavrnusSpawnPropertyHelpers::GetExposeOnSpawnProperties(CurrentClass);
    }
    // Also check cached properties in case class is not available
    if (CurrentExposeProperties.Num() == 0 && CachedProperties.Num() > 0)
    {
        CurrentExposeProperties = CachedProperties;
    }
    
    // Build a set of ExposeOnSpawn property names
    TSet<FName> ExposeOnSpawnPinNames;
    for (const FCavrnusExposeOnSpawnProperty& ExposeProp : CurrentExposeProperties)
    {
        ExposeOnSpawnPinNames.Add(FName(*ExposeProp.PropertyName));
    }
    
    // Also check cached properties in case we're removing pins from a previous class
    for (const FCavrnusExposeOnSpawnProperty& ExposeProp : CachedProperties)
    {
        ExposeOnSpawnPinNames.Add(FName(*ExposeProp.PropertyName));
    }
    
    // Find all pins that match ExposeOnSpawn property names
    for (UEdGraphPin* Pin : Node->Pins)
    {
        if (ExposeOnSpawnPinNames.Contains(Pin->PinName))
        {
            PinsToRemove.Add(Pin);
        }
    }
    
    for (UEdGraphPin* Pin : PinsToRemove)
    {
        Node->RemovePin(Pin);
    }
}

void FCavrnusNodeHelpers::CreateExposeOnSpawnPins(
    UK2Node* Node,
    UClass* CurrentClass,
    UClass*& CachedClass,
    TArray<FCavrnusExposeOnSpawnProperty>& CachedProperties,
    TFunction<UClass*()> GetClassFunc,
    TFunction<FEdGraphPinType(FProperty*)> GetPinTypeFunc,
    int32 InsertIndex)
{
    if (!Node)
    {
        return;
    }

    UClass* ClassToUse = CurrentClass;
    
    // If we can't get the class from the pin, try using cached class and properties
    // This happens during node refresh when the pin's DefaultObject isn't loaded yet
    if (!ClassToUse)
    {
        // Use cached class and properties if available
        if (CachedClass && CachedProperties.Num() > 0)
        {
            ClassToUse = CachedClass;
            
            // Re-fetch Property pointers from cached class if they're null (Property pointers aren't serialized)
            bool bNeedToRefreshProperties = false;
            for (const FCavrnusExposeOnSpawnProperty& ExposeProp : CachedProperties)
            {
                if (!ExposeProp.Property)
                {
                    bNeedToRefreshProperties = true;
                    break;
                }
            }
            
            if (bNeedToRefreshProperties)
            {
                // Re-fetch properties to get valid Property pointers
                CachedProperties = FCavrnusSpawnPropertyHelpers::GetExposeOnSpawnProperties(CachedClass);
            }
        }
        else
        {
            // No cached class or properties available, can't create pins
            return;
        }
    }
    else
    {
        // We have a valid class from the pin, fetch/update cached properties
        CachedProperties = FCavrnusSpawnPropertyHelpers::GetExposeOnSpawnProperties(ClassToUse);
        CachedClass = ClassToUse;
    }

    if (CachedProperties.Num() == 0)
    {
        return;
    }

    // Create all ExposeOnSpawn pins, preserving existing pins and their connections
    TArray<UEdGraphPin*> CreatedPins;
    
    for (const FCavrnusExposeOnSpawnProperty& ExposeProp : CachedProperties)
    {
        if (!ExposeProp.Property)
        {
            continue;
        }
        
        FName PropName = FName(*ExposeProp.PropertyName);
        FEdGraphPinType PinType = GetPinTypeFunc(ExposeProp.Property);
        if (PinType.PinCategory == NAME_None)
        {
            continue;
        }
        
        // Check if pin already exists - if so, preserve it (including connections and values)
        UEdGraphPin* ExistingPin = Node->FindPin(PropName);
        if (ExistingPin)
        {
            // Pin exists, check if type matches
            if (ExistingPin->PinType == PinType)
            {
                // Type matches, just update metadata and preserve the pin
                ExistingPin->PinFriendlyName = FText::FromString(ExposeProp.PropertyName);
                ExistingPin->PinToolTip = FString::Printf(TEXT("ExposeOnSpawn property: %s"), *ExposeProp.PropertyName);
                CreatedPins.Add(ExistingPin);
                continue;
            }
            else
            {
                // Type changed, we need to recreate but preserve connections if possible
                // Store connections before removing
                TArray<UEdGraphPin*> LinkedTo = ExistingPin->LinkedTo;
                FString PreservedDefaultValue = ExistingPin->DefaultValue;
                UObject* PreservedDefaultObject = ExistingPin->DefaultObject;
                
                // Remove old pin
                Node->RemovePin(ExistingPin);
                
                // Create new pin with correct type
                UEdGraphPin* NewPin = Node->CreatePin(EGPD_Input, PinType, PropName);
                if (NewPin)
                {
                    NewPin->PinFriendlyName = FText::FromString(ExposeProp.PropertyName);
                    NewPin->PinToolTip = FString::Printf(TEXT("ExposeOnSpawn property: %s"), *ExposeProp.PropertyName);
                    
                    // Restore default values
                    NewPin->DefaultValue = PreservedDefaultValue;
                    NewPin->DefaultObject = PreservedDefaultObject;
                    
                    // Restore connections if types are compatible
                    for (UEdGraphPin* LinkedPin : LinkedTo)
                    {
                        if (LinkedPin && LinkedPin->PinType == PinType)
                        {
                            NewPin->MakeLinkTo(LinkedPin);
                        }
                    }
                    
                    CreatedPins.Add(NewPin);
                }
            }
        }
        else
        {
            // Pin doesn't exist, create it
            UEdGraphPin* NewPin = Node->CreatePin(EGPD_Input, PinType, PropName);
            if (NewPin)
            {
                NewPin->PinFriendlyName = FText::FromString(ExposeProp.PropertyName);
                NewPin->PinToolTip = FString::Printf(TEXT("ExposeOnSpawn property: %s"), *ExposeProp.PropertyName);
                CreatedPins.Add(NewPin);
            }
            else
            {
                UE_LOG(LogCavrnusBlueprintModule, Error, TEXT("[CreateExposeOnSpawnPins] Failed to create pin for property: %s"), 
                    *ExposeProp.PropertyName);
            }
        }
    }
    
    // Move all created pins to the correct position
    MovePinsToPosition(Node, CreatedPins, InsertIndex);
}

void FCavrnusNodeHelpers::MovePinsToPosition(
    UK2Node* Node,
    const TArray<UEdGraphPin*>& PinsToMove,
    int32 InsertIndex)
{
    if (!Node || InsertIndex == INDEX_NONE || InsertIndex < 0 || PinsToMove.Num() == 0)
    {
        return;
    }

    // Move pins in reverse order to maintain their relative order
    for (int32 i = PinsToMove.Num() - 1; i >= 0; i--)
    {
        UEdGraphPin* PinToMove = PinsToMove[i];
        int32 CurrentIndex = Node->Pins.IndexOfByKey(PinToMove);
        if (CurrentIndex != INDEX_NONE)
        {
            // Calculate the target index (accounting for pins already moved)
            int32 TargetIndex = InsertIndex + (PinsToMove.Num() - 1 - i);
            
            // Only move if not already in the right position
            if (CurrentIndex != TargetIndex)
            {
                // Remove pin from current position
                Node->Pins.RemoveAt(CurrentIndex);
                
                // Adjust target index if we removed a pin before it
                if (CurrentIndex < TargetIndex)
                {
                    TargetIndex--;
                }
                
                // Insert at target position
                Node->Pins.Insert(PinToMove, TargetIndex);
            }
        }
    }
}

void FCavrnusNodeHelpers::ExpandExposeOnSpawnProperties(
    FKismetCompilerContext& CompilerContext,
    UEdGraph* SourceGraph,
    UK2Node* SourceNode,
    UEdGraphPin* ReturnPin,
    UEdGraphPin* SpaceConnectionPin,
    const TArray<FCavrnusExposeOnSpawnProperty>& ExposeProps)
{
    if (!SourceNode || !ReturnPin || !SpaceConnectionPin)
    {
        return;
    }

    // For each ExposeOnSpawn property, set its value on the actor and post to journal
    for (const FCavrnusExposeOnSpawnProperty& ExposeProp : ExposeProps)
    {
        if (!ExposeProp.Property)
        {
            continue;
        }

        // Get the ExposeOnSpawn property pin
        UEdGraphPin* PropertyPin = SourceNode->FindPin(ExposeProp.PropertyName);
        if (!PropertyPin)
        {
            continue;
        }

        // Skip if pin has no connection and no default value (user didn't set it)
        if (PropertyPin->LinkedTo.Num() == 0 && PropertyPin->DefaultValue.IsEmpty() && 
            PropertyPin->DefaultObject == nullptr)
        {
            continue;
        }

        UK2Node_CallFunction* GetComponentNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(SourceNode, SourceGraph);
        GetComponentNode->FunctionReference.SetExternalMember(
#if (ENGINE_MAJOR_VERSION == 5) && (ENGINE_MINOR_VERSION >= 2)
            // UE 5.2+: GetComponentByClass is overloaded, use string literal
            TEXT("GetComponentByClass"),
#else
            // UE 5.0-5.1: Use macro for compile-time checking
            GET_FUNCTION_NAME_CHECKED(AActor, GetComponentByClass),
#endif
            AActor::StaticClass()
        );
        GetComponentNode->AllocateDefaultPins();
        
        UEdGraphPin* GetComponentTargetPin = GetComponentNode->FindPin(UEdGraphSchema_K2::PN_Self);
        if (!GetComponentTargetPin)
        {
            GetComponentTargetPin = GetComponentNode->FindPin(TEXT("Target"));
        }
        UEdGraphPin* GetComponentClassPin = GetComponentNode->FindPin(TEXT("ComponentClass"));
        UEdGraphPin* GetComponentReturnPin = GetComponentNode->GetReturnValuePin();
        
        if (!GetComponentTargetPin || !GetComponentClassPin || !GetComponentReturnPin)
        {
            CompilerContext.MessageLog.Error(*FString::Printf(TEXT("Failed to find required pins in GetComponentByClass node for property '%s' on @@"), *ExposeProp.PropertyName), SourceNode);
            continue;
        }
        
        CompilerContext.CopyPinLinksToIntermediate(*ReturnPin, *GetComponentTargetPin);
        GetComponentClassPin->DefaultObject = UCavrnusPropertiesContainer::StaticClass();

        // Create a variable get node for ContainerName
        UK2Node_VariableGet* GetContainerNameVarNode = CompilerContext.SpawnIntermediateNode<UK2Node_VariableGet>(SourceNode, SourceGraph);
        FProperty* ContainerNameProp = FindFieldChecked<FProperty>(
            UCavrnusPropertiesContainer::StaticClass(), 
            GET_MEMBER_NAME_CHECKED(UCavrnusPropertiesContainer, ContainerName)
        );
        GetContainerNameVarNode->SetFromProperty(
            ContainerNameProp,
            false,
            UCavrnusPropertiesContainer::StaticClass()
        );
        GetContainerNameVarNode->AllocateDefaultPins();
        
        UEdGraphPin* VarGetSelfPin = GetContainerNameVarNode->FindPin(UEdGraphSchema_K2::PN_Self);
        if (!VarGetSelfPin)
        {
            CompilerContext.MessageLog.Error(*FString::Printf(TEXT("Failed to find Self pin in ContainerName variable get node for property '%s' on @@"), *ExposeProp.PropertyName), SourceNode);
            continue;
        }
        CompilerContext.CopyPinLinksToIntermediate(*GetComponentReturnPin, *VarGetSelfPin);
        
        UEdGraphPin* ContainerNamePin = GetContainerNameVarNode->FindPin(TEXT("ContainerName"));
        if (!ContainerNamePin)
        {
            CompilerContext.MessageLog.Error(*FString::Printf(TEXT("Failed to find ContainerName pin in variable get node for property '%s' on @@"), *ExposeProp.PropertyName), SourceNode);
            continue;
        }

        // Determine property type and create appropriate set/post function names
        FName SetFunctionName;
        FName PostFunctionName;
        UClass* FunctionClass = UCavrnusFunctionLibrary::StaticClass();
        
        FProperty* Prop = ExposeProp.Property;
        
        if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop))
        {
            SetFunctionName = GET_FUNCTION_NAME_CHECKED(UCavrnusFunctionLibrary, SetExposeOnSpawnBoolProperty);
            PostFunctionName = FName(TEXT("PostBoolPropertyUpdate"));
        }
        else if (FNumericProperty* NumericProp = CastField<FNumericProperty>(Prop))
        {
            SetFunctionName = GET_FUNCTION_NAME_CHECKED(UCavrnusFunctionLibrary, SetExposeOnSpawnFloatProperty);
            PostFunctionName = FName(TEXT("PostFloatPropertyUpdate"));
        }
        else if (FStrProperty* StrProp = CastField<FStrProperty>(Prop))
        {
            SetFunctionName = GET_FUNCTION_NAME_CHECKED(UCavrnusFunctionLibrary, SetExposeOnSpawnStringProperty);
            PostFunctionName = FName(TEXT("PostStringPropertyUpdate"));
        }
        else if (FTextProperty* TextProp = CastField<FTextProperty>(Prop))
        {
            // Text properties are handled as strings
            SetFunctionName = GET_FUNCTION_NAME_CHECKED(UCavrnusFunctionLibrary, SetExposeOnSpawnStringProperty);
            PostFunctionName = FName(TEXT("PostStringPropertyUpdate"));
        }
        else if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
        {
            UScriptStruct* Struct = StructProp->Struct;
            if (Struct)
            {
                if (Struct == TBaseStructure<FVector>::Get() || Struct == TBaseStructure<FVector4>::Get())
                {
                    SetFunctionName = GET_FUNCTION_NAME_CHECKED(UCavrnusFunctionLibrary, SetExposeOnSpawnVectorProperty);
                    PostFunctionName = FName(TEXT("PostVectorPropertyUpdate"));
                }
                else if (Struct == TBaseStructure<FTransform>::Get())
                {
                    SetFunctionName = GET_FUNCTION_NAME_CHECKED(UCavrnusFunctionLibrary, SetExposeOnSpawnTransformProperty);
                    PostFunctionName = FName(TEXT("PostTransformPropertyUpdate"));
                }
                else if (Struct == TBaseStructure<FLinearColor>::Get() || Struct == TBaseStructure<FColor>::Get())
                {
                    SetFunctionName = GET_FUNCTION_NAME_CHECKED(UCavrnusFunctionLibrary, SetExposeOnSpawnColorProperty);
                    PostFunctionName = FName(TEXT("PostColorPropertyUpdate"));
                }
                else
                {
                    // Arbitrary struct - use string
                    SetFunctionName = GET_FUNCTION_NAME_CHECKED(UCavrnusFunctionLibrary, SetExposeOnSpawnStringProperty);
                    PostFunctionName = FName(TEXT("PostStringPropertyUpdate"));
                }
            }
        }
        else
        {
            continue; // Unsupported property type
        }

        // Step 1: Set the property value on the actor (like Unreal's SpawnActor does)
        UK2Node_CallFunction* SetPropertyNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(SourceNode, SourceGraph);
        SetPropertyNode->FunctionReference.SetExternalMember(SetFunctionName, FunctionClass);
        SetPropertyNode->AllocateDefaultPins();

        // Connect Actor
        UEdGraphPin* SetActorPin = SetPropertyNode->FindPin(TEXT("Actor"));
        if (!SetActorPin)
        {
            CompilerContext.MessageLog.Error(*FString::Printf(TEXT("Failed to find Actor pin in SetExposeOnSpawn property node for property '%s' on @@"), *ExposeProp.PropertyName), SourceNode);
            continue;
        }
        CompilerContext.CopyPinLinksToIntermediate(*ReturnPin, *SetActorPin);

        // Connect PropertyName
        UEdGraphPin* SetPropertyNamePin = SetPropertyNode->FindPin(TEXT("PropertyName"));
        if (!SetPropertyNamePin)
        {
            CompilerContext.MessageLog.Error(*FString::Printf(TEXT("Failed to find PropertyName pin in SetExposeOnSpawn property node for property '%s' on @@"), *ExposeProp.PropertyName), SourceNode);
            continue;
        }
        SetPropertyNamePin->DefaultValue = ExposeProp.PropertyName;

        // Connect Value from the ExposeOnSpawn pin (copy, not move, since we need it again)
        UEdGraphPin* SetValuePin = SetPropertyNode->FindPin(TEXT("Value"));
        if (!SetValuePin)
        {
            CompilerContext.MessageLog.Error(*FString::Printf(TEXT("Failed to find Value pin in SetExposeOnSpawn property node for property '%s' on @@"), *ExposeProp.PropertyName), SourceNode);
            continue;
        }
        CompilerContext.CopyPinLinksToIntermediate(*PropertyPin, *SetValuePin);

        // Step 2: Post the property value to the journal
        UK2Node_CallFunction* PostPropertyNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(SourceNode, SourceGraph);
        PostPropertyNode->FunctionReference.SetExternalMember(PostFunctionName, FunctionClass);
        PostPropertyNode->AllocateDefaultPins();

        // Connect SpaceConnection (copy, not move, since it's used in the loop)
        UEdGraphPin* PostSpaceConnectionPin = PostPropertyNode->FindPin(TEXT("SpaceConnection"));
        if (!PostSpaceConnectionPin)
        {
            CompilerContext.MessageLog.Error(*FString::Printf(TEXT("Failed to find SpaceConnection pin in Post property node for property '%s' on @@"), *ExposeProp.PropertyName), SourceNode);
            continue;
        }
        CompilerContext.CopyPinLinksToIntermediate(*SpaceConnectionPin, *PostSpaceConnectionPin);

        // Connect ContainerName (InstanceId) (copy, not move, since it's used in the loop)
        UEdGraphPin* PostContainerNamePin = PostPropertyNode->FindPin(TEXT("ContainerName"));
        if (!PostContainerNamePin)
        {
            CompilerContext.MessageLog.Error(*FString::Printf(TEXT("Failed to find ContainerName pin in Post property node for property '%s' on @@"), *ExposeProp.PropertyName), SourceNode);
            continue;
        }
        CompilerContext.CopyPinLinksToIntermediate(*ContainerNamePin, *PostContainerNamePin);

        // Connect PropertyName
        UEdGraphPin* PostPropertyNamePin = PostPropertyNode->FindPin(TEXT("PropertyName"));
        if (!PostPropertyNamePin)
        {
            CompilerContext.MessageLog.Error(*FString::Printf(TEXT("Failed to find PropertyName pin in Post property node for property '%s' on @@"), *ExposeProp.PropertyName), SourceNode);
            continue;
        }
        PostPropertyNamePin->DefaultValue = ExposeProp.PropertyName;

        // Connect PropertyValue from the ExposeOnSpawn pin (copy again, since we copied to SetValuePin)
        UEdGraphPin* PostPropertyValuePin = PostPropertyNode->FindPin(TEXT("PropertyValue"));
        if (!PostPropertyValuePin)
        {
            CompilerContext.MessageLog.Error(*FString::Printf(TEXT("Failed to find PropertyValue pin in Post property node for property '%s' on @@"), *ExposeProp.PropertyName), SourceNode);
            continue;
        }
        CompilerContext.CopyPinLinksToIntermediate(*PropertyPin, *PostPropertyValuePin);
    }
}

bool FCavrnusNodeHelpers::BuildExposeOnSpawnPropertyArray(
    FKismetCompilerContext& CompilerContext,
    UEdGraph* SourceGraph,
    UK2Node* SourceNode,
    const TArray<FCavrnusExposeOnSpawnProperty>& ExposeProps,
    TArray<UK2Node_MakeStruct*>& OutMakeStructNodes,
    UK2Node_MakeArray*& OutMakeArrayNode)
{
    OutMakeStructNodes.Empty();
    OutMakeArrayNode = nullptr;

    if (!SourceNode || !SourceGraph)
    {
        return false;
    }

    // Build array of FCavrnusSpawnPropertyValue from ExposeOnSpawn pins
    for (const FCavrnusExposeOnSpawnProperty& ExposeProp : ExposeProps)
    {
        if (!ExposeProp.Property)
        {
            continue;
        }

        // Get the ExposeOnSpawn property pin
        UEdGraphPin* PropertyPin = SourceNode->FindPin(ExposeProp.PropertyName);
        if (!PropertyPin)
        {
            UE_LOG(LogCavrnusBlueprintModule, Warning, TEXT("BuildExposeOnSpawnPropertyArray: Could not find pin for property '%s'"), 
                *ExposeProp.PropertyName);
            continue;
        }

        // Create a MakeStruct node for FCavrnusSpawnPropertyValue
        UK2Node_MakeStruct* MakeStructNode = CompilerContext.SpawnIntermediateNode<UK2Node_MakeStruct>(SourceNode, SourceGraph);
        MakeStructNode->StructType = FCavrnusSpawnPropertyValue::StaticStruct();
        MakeStructNode->AllocateDefaultPins();

        // Set PropertyName
        UEdGraphPin* PropertyNamePin = MakeStructNode->FindPin(TEXT("PropertyName"));
        if (PropertyNamePin)
        {
            PropertyNamePin->DefaultValue = ExposeProp.PropertyName;
        }

        // Determine property type and set appropriate value field
        FProperty* Prop = ExposeProp.Property;
        ECavrnusSpawnPropertyType PropertyType = ECavrnusSpawnPropertyType::Bool;
        
        if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop))
        {
            PropertyType = ECavrnusSpawnPropertyType::Bool;
            UEdGraphPin* PropertyTypePin = MakeStructNode->FindPin(TEXT("PropertyType"));
            if (PropertyTypePin)
            {
                const UEnum* TypeEnum = StaticEnum<ECavrnusSpawnPropertyType>();
                PropertyTypePin->DefaultValue = TypeEnum->GetNameStringByValue(static_cast<int64>(PropertyType));
            }
            
            UEdGraphPin* BoolValuePin = MakeStructNode->FindPin(TEXT("BoolValue"));
            if (BoolValuePin)
            {
                CompilerContext.MovePinLinksToIntermediate(*PropertyPin, *BoolValuePin);
                if (PropertyPin->LinkedTo.Num() == 0 && !PropertyPin->DefaultValue.IsEmpty())
                {
                    BoolValuePin->DefaultValue = PropertyPin->DefaultValue;
                }
            }
        }
        else if (FNumericProperty* NumericProp = CastField<FNumericProperty>(Prop))
        {
            PropertyType = ECavrnusSpawnPropertyType::Float;
            UEdGraphPin* PropertyTypePin = MakeStructNode->FindPin(TEXT("PropertyType"));
            if (PropertyTypePin)
            {
                const UEnum* TypeEnum = StaticEnum<ECavrnusSpawnPropertyType>();
                PropertyTypePin->DefaultValue = TypeEnum->GetNameStringByValue(static_cast<int64>(PropertyType));
            }
            
            UEdGraphPin* FloatValuePin = MakeStructNode->FindPin(TEXT("FloatValue"));
            if (FloatValuePin)
            {
                CompilerContext.MovePinLinksToIntermediate(*PropertyPin, *FloatValuePin);
                if (PropertyPin->LinkedTo.Num() == 0 && !PropertyPin->DefaultValue.IsEmpty())
                {
                    FloatValuePin->DefaultValue = PropertyPin->DefaultValue;
                }
            }
        }
        else if (FStrProperty* StrProp = CastField<FStrProperty>(Prop))
        {
            PropertyType = ECavrnusSpawnPropertyType::String;
            UEdGraphPin* PropertyTypePin = MakeStructNode->FindPin(TEXT("PropertyType"));
            if (PropertyTypePin)
            {
                const UEnum* TypeEnum = StaticEnum<ECavrnusSpawnPropertyType>();
                PropertyTypePin->DefaultValue = TypeEnum->GetNameStringByValue(static_cast<int64>(PropertyType));
            }
            
            UEdGraphPin* StringValuePin = MakeStructNode->FindPin(TEXT("StringValue"));
            if (StringValuePin)
            {
                CompilerContext.MovePinLinksToIntermediate(*PropertyPin, *StringValuePin);
                if (PropertyPin->LinkedTo.Num() == 0 && !PropertyPin->DefaultValue.IsEmpty())
                {
                    StringValuePin->DefaultValue = PropertyPin->DefaultValue;
                }
            }
        }
        else if (FTextProperty* TextProp = CastField<FTextProperty>(Prop))
        {
            PropertyType = ECavrnusSpawnPropertyType::String;
            UEdGraphPin* PropertyTypePin = MakeStructNode->FindPin(TEXT("PropertyType"));
            if (PropertyTypePin)
            {
                const UEnum* TypeEnum = StaticEnum<ECavrnusSpawnPropertyType>();
                PropertyTypePin->DefaultValue = TypeEnum->GetNameStringByValue(static_cast<int64>(PropertyType));
            }
            
            UEdGraphPin* StringValuePin = MakeStructNode->FindPin(TEXT("StringValue"));
            if (StringValuePin)
            {
                CompilerContext.MovePinLinksToIntermediate(*PropertyPin, *StringValuePin);
                if (PropertyPin->LinkedTo.Num() == 0 && !PropertyPin->DefaultValue.IsEmpty())
                {
                    StringValuePin->DefaultValue = PropertyPin->DefaultValue;
                }
            }
        }
        else if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
        {
            UScriptStruct* Struct = StructProp->Struct;
            if (Struct)
            {
                if (Struct == TBaseStructure<FVector>::Get() || Struct == TBaseStructure<FVector4>::Get())
                {
                    PropertyType = ECavrnusSpawnPropertyType::Vector;
                    UEdGraphPin* PropertyTypePin = MakeStructNode->FindPin(TEXT("PropertyType"));
                    if (PropertyTypePin)
                    {
                        const UEnum* TypeEnum = StaticEnum<ECavrnusSpawnPropertyType>();
                        PropertyTypePin->DefaultValue = TypeEnum->GetNameStringByValue(static_cast<int64>(PropertyType));
                    }
                    
                    UEdGraphPin* VectorValuePin = MakeStructNode->FindPin(TEXT("VectorValue"));
                    if (VectorValuePin)
                    {
                        CompilerContext.MovePinLinksToIntermediate(*PropertyPin, *VectorValuePin);
                        if (PropertyPin->LinkedTo.Num() == 0 && !PropertyPin->DefaultValue.IsEmpty())
                        {
                            VectorValuePin->DefaultValue = PropertyPin->DefaultValue;
                        }
                    }
                }
                else if (Struct == TBaseStructure<FTransform>::Get())
                {
                    PropertyType = ECavrnusSpawnPropertyType::Transform;
                    UEdGraphPin* PropertyTypePin = MakeStructNode->FindPin(TEXT("PropertyType"));
                    if (PropertyTypePin)
                    {
                        const UEnum* TypeEnum = StaticEnum<ECavrnusSpawnPropertyType>();
                        PropertyTypePin->DefaultValue = TypeEnum->GetNameStringByValue(static_cast<int64>(PropertyType));
                    }
                    
                    UEdGraphPin* TransformValuePin = MakeStructNode->FindPin(TEXT("TransformValue"));
                    if (TransformValuePin)
                    {
                        CompilerContext.MovePinLinksToIntermediate(*PropertyPin, *TransformValuePin);
                        if (PropertyPin->LinkedTo.Num() == 0 && !PropertyPin->DefaultValue.IsEmpty())
                        {
                            TransformValuePin->DefaultValue = PropertyPin->DefaultValue;
                        }
                    }
                }
                else if (Struct == TBaseStructure<FLinearColor>::Get() || Struct == TBaseStructure<FColor>::Get())
                {
                    PropertyType = ECavrnusSpawnPropertyType::Color;
                    UEdGraphPin* PropertyTypePin = MakeStructNode->FindPin(TEXT("PropertyType"));
                    if (PropertyTypePin)
                    {
                        const UEnum* TypeEnum = StaticEnum<ECavrnusSpawnPropertyType>();
                        PropertyTypePin->DefaultValue = TypeEnum->GetNameStringByValue(static_cast<int64>(PropertyType));
                    }
                    
                    UEdGraphPin* ColorValuePin = MakeStructNode->FindPin(TEXT("ColorValue"));
                    if (ColorValuePin)
                    {
                        CompilerContext.MovePinLinksToIntermediate(*PropertyPin, *ColorValuePin);
                        if (PropertyPin->LinkedTo.Num() == 0 && !PropertyPin->DefaultValue.IsEmpty())
                        {
                            ColorValuePin->DefaultValue = PropertyPin->DefaultValue;
                        }
                    }
                }
                else
                {
                    // Arbitrary struct - serialize to string
                    PropertyType = ECavrnusSpawnPropertyType::Struct;
                    UEdGraphPin* PropertyTypePin = MakeStructNode->FindPin(TEXT("PropertyType"));
                    if (PropertyTypePin)
                    {
                        const UEnum* TypeEnum = StaticEnum<ECavrnusSpawnPropertyType>();
                        PropertyTypePin->DefaultValue = TypeEnum->GetNameStringByValue(static_cast<int64>(PropertyType));
                    }
                    
                    UEdGraphPin* StringValuePin = MakeStructNode->FindPin(TEXT("StringValue"));
                    if (StringValuePin)
                    {
                        bool bIsStructPin = PropertyPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Struct;
                        
                        if (bIsStructPin)
                        {
                            if (PropertyPin->LinkedTo.Num() > 0)
                            {
                                // Connected struct pin - cannot automatically convert
                                CompilerContext.MessageLog.Error(*FString::Printf(
                                    TEXT("Connected struct pin '%s' on @@ cannot be automatically converted to string. "
                                         "Please use a default value for the struct property, or manually serialize it to a string before connecting."), 
                                    *ExposeProp.PropertyName), SourceNode);
                                continue;
                            }
                            else
                            {
                                // Default value on struct pin
                                if (!PropertyPin->DefaultValue.IsEmpty())
                                {
                                    StringValuePin->DefaultValue = PropertyPin->DefaultValue;
                                }
                            }
                        }
                        else
                        {
                            // Pin is already a string type
                            CompilerContext.MovePinLinksToIntermediate(*PropertyPin, *StringValuePin);
                            if (PropertyPin->LinkedTo.Num() == 0 && !PropertyPin->DefaultValue.IsEmpty())
                            {
                                StringValuePin->DefaultValue = PropertyPin->DefaultValue;
                            }
                        }
                    }
                }
            }
        }

        OutMakeStructNodes.Add(MakeStructNode);
    }
    
    // Create MakeArray node to combine all the struct values
    if (OutMakeStructNodes.Num() > 0)
    {
        OutMakeArrayNode = CompilerContext.SpawnIntermediateNode<UK2Node_MakeArray>(SourceNode, SourceGraph);
        
        // Set up the array pin type for FCavrnusSpawnPropertyValue
        FEdGraphPinType ArrayPinType;
        ArrayPinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
        ArrayPinType.PinSubCategoryObject = FCavrnusSpawnPropertyValue::StaticStruct();
        ArrayPinType.ContainerType = EPinContainerType::Array;
        
        // Allocate default pins first
        OutMakeArrayNode->AllocateDefaultPins();
        
        // Get the inner type (without container) for input pins
        FEdGraphPinType InnerType = ArrayPinType;
        InnerType.ContainerType = EPinContainerType::None;

        // Set the inner type by finding the wildcard pin and setting its type
        UEdGraphPin* WildcardPin = nullptr;
        for (UEdGraphPin* Pin : OutMakeArrayNode->Pins)
        {
            if (Pin && Pin->Direction == EGPD_Input && Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard)
            {
                WildcardPin = Pin;
                Pin->PinType = InnerType;
                break;
            }
        }
        
        // Also set the output pin type
        for (UEdGraphPin* Pin : OutMakeArrayNode->Pins)
        {
            if (Pin && Pin->Direction == EGPD_Output)
            {
                Pin->PinType = ArrayPinType;
                break;
            }
        }

        // Add input pins for each MakeStruct node
        TArray<UEdGraphPin*> ArrayInputPins;
        
        if (WildcardPin)
        {
            // Use the existing wildcard pin for the first element
            ArrayInputPins.Add(WildcardPin);
            
            // Create additional input pins for remaining elements
            for (int32 i = 1; i < OutMakeStructNodes.Num(); ++i)
            {
                FString PinName = FString::Printf(TEXT("[%d]"), i);
                
                UEdGraphPin* NewInputPin = OutMakeArrayNode->CreatePin(EGPD_Input, InnerType, *PinName);
                if (NewInputPin)
                {
                    ArrayInputPins.Add(NewInputPin);
                }
                else
                {
                    // Try alternative pin name formats
                    FString AltPinName1 = FString::Printf(TEXT("%d"), i);
                    FString AltPinName2 = FString::Printf(TEXT("Element %d"), i);
                    
                    UEdGraphPin* ExistingPin = OutMakeArrayNode->FindPin(AltPinName1);
                    if (!ExistingPin)
                    {
                        ExistingPin = OutMakeArrayNode->FindPin(AltPinName2);
                    }
                    if (!ExistingPin)
                    {
                        ExistingPin = OutMakeArrayNode->FindPin(PinName);
                    }
                    
                    if (ExistingPin)
                    {
                        ArrayInputPins.Add(ExistingPin);
                    }
                }
            }
        }
        else
        {
            // No wildcard pin found - create all input pins
            for (int32 i = 0; i < OutMakeStructNodes.Num(); ++i)
            {
                FString PinName = FString::Printf(TEXT("[%d]"), i);
                UEdGraphPin* NewInputPin = OutMakeArrayNode->CreatePin(EGPD_Input, InnerType, *PinName);
                if (NewInputPin)
                {
                    ArrayInputPins.Add(NewInputPin);
                }
                else
                {
                    // Try finding existing pin with alternative names
                    FString AltPinName1 = FString::Printf(TEXT("%d"), i);
                    FString AltPinName2 = FString::Printf(TEXT("Element %d"), i);
                    
                    UEdGraphPin* ExistingPin = OutMakeArrayNode->FindPin(PinName);
                    if (!ExistingPin)
                    {
                        ExistingPin = OutMakeArrayNode->FindPin(AltPinName1);
                    }
                    if (!ExistingPin)
                    {
                        ExistingPin = OutMakeArrayNode->FindPin(AltPinName2);
                    }
                    
                    if (ExistingPin)
                    {
                        ArrayInputPins.Add(ExistingPin);
                    }
                }
            }
        }
        
        // Connect all MakeStruct nodes to the MakeArray node
        for (int32 i = 0; i < OutMakeStructNodes.Num() && i < ArrayInputPins.Num(); ++i)
        {
            // Find the output pin from the MakeStruct node
            UEdGraphPin* StructOutputPin = nullptr;
            for (UEdGraphPin* Pin : OutMakeStructNodes[i]->Pins)
            {
                if (Pin && Pin->Direction == EGPD_Output)
                {
                    StructOutputPin = Pin;
                    break;
                }
            }
            
            if (StructOutputPin && ArrayInputPins[i])
            {
                StructOutputPin->MakeLinkTo(ArrayInputPins[i]);
            }
        }
    }

    return true;
}

bool FCavrnusNodeHelpers::WireFunctionCallNode(
    FKismetCompilerContext& CompilerContext,
    UK2Node_CallFunction* FunctionCallNode,
    const TMap<FString, UEdGraphPin*>& ParameterPinMappings,
    UEdGraphPin* ExecutePin,
    UEdGraphPin* ThenPin,
    UEdGraphPin* ReturnPin)
{
    if (!FunctionCallNode || !ExecutePin || !ThenPin)
    {
        return false;
    }

    // Wire execution pins
    UEdGraphPin* FunctionExecutePin = FunctionCallNode->FindPin(UEdGraphSchema_K2::PN_Execute);
    if (FunctionExecutePin && ExecutePin)
    {
        CompilerContext.MovePinLinksToIntermediate(*ExecutePin, *FunctionExecutePin);
    }

    UEdGraphPin* FunctionThenPin = FunctionCallNode->FindPin(UEdGraphSchema_K2::PN_Then);
    if (FunctionThenPin && ThenPin)
    {
        CompilerContext.MovePinLinksToIntermediate(*ThenPin, *FunctionThenPin);
    }

    // Wire parameter pins
    for (const auto& PinMapping : ParameterPinMappings)
    {
        const FString& ParamName = PinMapping.Key;
        UEdGraphPin* SourcePin = PinMapping.Value;
        
        if (!SourcePin)
        {
            continue;
        }

        UEdGraphPin* FunctionParamPin = FunctionCallNode->FindPin(ParamName);
        if (FunctionParamPin)
        {
            CompilerContext.MovePinLinksToIntermediate(*SourcePin, *FunctionParamPin);
            
            // Also copy default values for pins that support them
            if (SourcePin->LinkedTo.Num() == 0)
            {
                if (!SourcePin->DefaultValue.IsEmpty())
                {
                    FunctionParamPin->DefaultValue = SourcePin->DefaultValue;
                }
                if (SourcePin->DefaultObject)
                {
                    FunctionParamPin->DefaultObject = SourcePin->DefaultObject;
                }
            }
        }
    }

    // Wire return value if provided
    if (ReturnPin)
    {
        UEdGraphPin* FunctionReturnPin = FunctionCallNode->GetReturnValuePin();
        if (FunctionReturnPin)
        {
            CompilerContext.MovePinLinksToIntermediate(*FunctionReturnPin, *ReturnPin);
        }
    }

    return true;
}

