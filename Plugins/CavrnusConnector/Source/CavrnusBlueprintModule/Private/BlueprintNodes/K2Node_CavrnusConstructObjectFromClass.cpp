// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "BlueprintNodes/K2Node_CavrnusConstructObjectFromClass.h"
#include "BlueprintNodes/CavrnusNodeHelpers.h"
#include "Managers/SpawnedObjects/SpawnedObjectsManager.h"
#include "Managers/SpawnedObjects/CavrnusSpawnPropertyHelpers.h"
#include "AssetManager/DataAssets/CavrnusSpawnableRegistryDataAsset.h"
#include "AssetManager/CavrnusDataAssetManager.h"
#include "Core/Subsystems/CavrnusSubsystem.h"
#include "Core/Contexts/CavrnusEditorContext.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "CavrnusFunctionLibrary.h"
#include "CavrnusBlueprintModule.h"
#include "KismetCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraphSchema_K2.h"
#include "UObject/UnrealType.h"
#include "Engine/Engine.h"
#include "K2Node_CallFunction.h"
#include "EdGraph/EdGraphSchema.h"
#include "KismetCompiler.h"
#include "Managers/SpawnedObjects/CavrnusSpawnPropertyValue.h"
#include "K2Node_MakeStruct.h"
#include "K2Node_MakeArray.h"

UK2Node_CavrnusConstructObjectFromClass::UK2Node_CavrnusConstructObjectFromClass(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , CachedObjectClass(nullptr)
{
}

void UK2Node_CavrnusConstructObjectFromClass::AllocateDefaultPins()
{
    // Create execution pins
    CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
    CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);

    // Create return value pin (UObject*)
    FEdGraphPinType ReturnPinType;
    ReturnPinType.PinCategory = UEdGraphSchema_K2::PC_Object;
    ReturnPinType.PinSubCategoryObject = UObject::StaticClass();
    CreatePin(EGPD_Output, ReturnPinType, UEdGraphSchema_K2::PN_ReturnValue);

    // Manually create parameter pins (since we're not using a UFUNCTION)
    const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();

    // SpaceConnection pin
    FEdGraphPinType SpaceConnectionPinType;
    SpaceConnectionPinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
    SpaceConnectionPinType.PinSubCategoryObject = FCavrnusSpaceConnection::StaticStruct();
    UEdGraphPin* SpaceConnectionPin = CreatePin(EGPD_Input, SpaceConnectionPinType, TEXT("SpaceConnection"));
    if (SpaceConnectionPin)
    {
        SpaceConnectionPin->PinFriendlyName = FText::FromString(TEXT("Space Connection"));
        SpaceConnectionPin->PinToolTip = TEXT("The space connection where the object will be constructed");
    }

    // DataAsset pin
    FEdGraphPinType DataAssetPinType;
    DataAssetPinType.PinCategory = UEdGraphSchema_K2::PC_Object;
    DataAssetPinType.PinSubCategoryObject = UCavrnusSpawnableRegistryDataAsset::StaticClass();
    UEdGraphPin* DataAssetPin = CreatePin(EGPD_Input, DataAssetPinType, TEXT("DataAsset"));
    if (DataAssetPin)
    {
        DataAssetPin->PinFriendlyName = FText::FromString(TEXT("Data Asset"));
        DataAssetPin->PinToolTip = TEXT("Optional DataAsset to use for validation (defaults to main one)");
    }

    // ObjectClass pin
    FEdGraphPinType ObjectClassPinType;
    ObjectClassPinType.PinCategory = UEdGraphSchema_K2::PC_Class;
    ObjectClassPinType.PinSubCategoryObject = UObject::StaticClass();
    UEdGraphPin* ObjectClassPin = CreatePin(EGPD_Input, ObjectClassPinType, TEXT("ObjectClass"));
    if (ObjectClassPin)
    {
        ObjectClassPin->PinFriendlyName = FText::FromString(TEXT("Class"));
        ObjectClassPin->PinToolTip = TEXT("The class of object to construct (filtered by selected DataAsset)");
    }

    // Update ObjectClass pin filtering
    UpdateObjectClassPinFiltering();

    // Cache class and create ExposeOnSpawn pins based on it
    // Only update CachedObjectClass if we get a valid class - preserve existing cached value during refresh
    UClass* ObjectClassFromPin = GetObjectClassFromPin();
    if (ObjectClassFromPin)
    {
        CachedObjectClass = ObjectClassFromPin;
    }
    // If GetObjectClassFromPin() returns nullptr, preserve existing CachedObjectClass (if any)
    // This allows CreateExposeOnSpawnPins() to use cached properties during node refresh
    
    CreateExposeOnSpawnPins();
    
    // Ensure CachedExposeOnSpawnProperties is populated
    if (CachedExposeOnSpawnProperties.Num() == 0 && CachedObjectClass)
    {
        CachedExposeOnSpawnProperties = FCavrnusSpawnPropertyHelpers::GetExposeOnSpawnProperties(CachedObjectClass);
    }
}

void UK2Node_CavrnusConstructObjectFromClass::NotifyPinConnectionListChanged(UEdGraphPin* Pin)
{
    // If ObjectClass pin changed, update ExposeOnSpawn pins
    if (Pin && Pin->PinName == TEXT("ObjectClass"))
    {
        RemoveExposeOnSpawnPins();
        CreateExposeOnSpawnPins();
        CachedObjectClass = GetObjectClassFromPin();
    }
    // If DataAsset pin changed, update ObjectClass pin filtering and clear ObjectClass
    else if (Pin && Pin->PinName == TEXT("DataAsset"))
    {
        // Clear the ObjectClass pin since the valid classes list has changed
        UEdGraphPin* ObjectClassPin = FindPin(TEXT("ObjectClass"));
        if (ObjectClassPin)
        {
            ObjectClassPin->DefaultObject = nullptr;
            ObjectClassPin->DefaultValue = TEXT("");
            
            // Remove ExposeOnSpawn pins since we no longer have a valid object class
            RemoveExposeOnSpawnPins();
            CachedObjectClass = nullptr;
            
            // Notify the graph that the node structure has changed
            if (UEdGraph* Graph = GetGraph())
            {
                Graph->NotifyGraphChanged();
                if (UBlueprint* BP = FBlueprintEditorUtils::FindBlueprintForGraph(Graph))
                {
                    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP);
                }
            }
        }
        
        UpdateObjectClassPinFiltering();
    }
}

void UK2Node_CavrnusConstructObjectFromClass::PostReconstructNode()
{
    // Update ObjectClass pin filtering based on current DataAsset
    UpdateObjectClassPinFiltering();

    // Check if ObjectClass has changed (default value, not just connection)
    UClass* CurrentObjectClass = GetObjectClassFromPin();
    
    // Only update pins if we have a valid class AND it's different from cached
    // If CurrentObjectClass is nullptr, preserve existing pins (they might be from a valid class that just isn't loaded yet)
    if (CurrentObjectClass && CurrentObjectClass != CachedObjectClass)
    {
        RemoveExposeOnSpawnPins();
        CreateExposeOnSpawnPins();
        CachedObjectClass = CurrentObjectClass;
    }
    // If we have a cached class but can't determine current class, ensure pins exist
    else if (CachedObjectClass && !CurrentObjectClass)
    {
        // Class might not be loaded yet, but we have cached properties - ensure pins exist
        if (CachedExposeOnSpawnProperties.Num() > 0)
        {
            // Check if pins exist, if not create them
            bool bAllPinsExist = true;
            for (const FCavrnusExposeOnSpawnProperty& ExposeProp : CachedExposeOnSpawnProperties)
            {
                if (!FindPin(FName(*ExposeProp.PropertyName)))
                {
                    bAllPinsExist = false;
                    break;
                }
            }
            
            if (!bAllPinsExist)
            {
                CreateExposeOnSpawnPins();
            }
        }
    }
    // If we have no class and no cache, remove pins
    else if (!CurrentObjectClass && !CachedObjectClass)
    {
        RemoveExposeOnSpawnPins();
    }
}

void UK2Node_CavrnusConstructObjectFromClass::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    // Get the function we want to override (use the WithArray version which is a UFUNCTION)
    UFunction* FunctionToRegister = USpawnedObjectsManager::StaticClass()->FindFunctionByName(TEXT("CavrnusConstructObjectFromClassWithArray"));
    if (!FunctionToRegister)
    {
        UE_LOG(LogCavrnusBlueprintModule, Error, TEXT("[GetMenuActions] CavrnusConstructObjectFromClassWithArray function not found!"));
        return;
    }

    // Register for the function (not the class) - this will replace the default UK2Node_CallFunction
    if (ActionRegistrar.IsOpenForRegistration(FunctionToRegister))
    {
        // Create a node spawner for our custom node class
        UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
        check(NodeSpawner != nullptr);

        // Set a custom menu name to distinguish from the default UK2Node_CallFunction entry
        NodeSpawner->DefaultMenuSignature.MenuName = FText::FromString(TEXT("Cavrnus Construct Object From Class"));
        NodeSpawner->DefaultMenuSignature.Category = FText::FromString(TEXT("Cavrnus|Objects"));
        NodeSpawner->DefaultMenuSignature.Tooltip = FText::FromString(TEXT("Constructs a UObject with ExposeOnSpawn property support."));

        // No need to set TargetFunction anymore since we manually create pins
        // The node will call CavrnusConstructObjectFromClassWithArray in ExpandNode

        // Register for the function - this should replace the default UK2Node_CallFunction
        ActionRegistrar.AddBlueprintAction(FunctionToRegister, NodeSpawner);
    }
}

void UK2Node_CavrnusConstructObjectFromClass::PinDefaultValueChanged(UEdGraphPin* Pin)
{
    if (!Pin)
    {
        return;
    }

    // If ObjectClass pin default value changed, update ExposeOnSpawn pins
    if (Pin->PinName == TEXT("ObjectClass"))
    {
        UClass* CurrentObjectClass = GetObjectClassFromPin();
        if (CurrentObjectClass == nullptr)
        {
            CachedObjectClass = nullptr;
            RemoveExposeOnSpawnPins();
        }
        else if (CurrentObjectClass != CachedObjectClass)
        {
            RemoveExposeOnSpawnPins();
            CreateExposeOnSpawnPins();
            CachedObjectClass = CurrentObjectClass;
        }
        // Notify the graph that the node structure has changed
        if (UEdGraph* Graph = GetGraph())
        {
            Graph->NotifyGraphChanged();
            if (UBlueprint* BP = FBlueprintEditorUtils::FindBlueprintForGraph(Graph))
            {
                FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP);
            }
        }
    }
    // If DataAsset pin changed, clear ObjectClass and update filtering
    else if (Pin->PinName == TEXT("DataAsset"))
    {
        // Clear the ObjectClass pin since the valid classes list has changed
        UEdGraphPin* ObjectClassPin = FindPin(TEXT("ObjectClass"));
        if (ObjectClassPin)
        {
            ObjectClassPin->DefaultObject = nullptr;
            ObjectClassPin->DefaultValue = TEXT("");
            
            // Remove ExposeOnSpawn pins since we no longer have a valid object class
            RemoveExposeOnSpawnPins();
            CachedObjectClass = nullptr;
            
            // Notify the graph that the node structure has changed
            if (UEdGraph* Graph = GetGraph())
            {
                Graph->NotifyGraphChanged();
                if (UBlueprint* BP = FBlueprintEditorUtils::FindBlueprintForGraph(Graph))
                {
                    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP);
                }
            }
        }
        
        UpdateObjectClassPinFiltering();
    }
}

void UK2Node_CavrnusConstructObjectFromClass::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
    // Get input pins
    UEdGraphPin* SpaceConnectionPin = FindPin(TEXT("SpaceConnection"));
    UEdGraphPin* ObjectClassPin = FindPin(TEXT("ObjectClass"));
    UEdGraphPin* DataAssetPin = FindPin(TEXT("DataAsset"));
    UEdGraphPin* OurExecutePin = FindPin(UEdGraphSchema_K2::PN_Execute);
    UEdGraphPin* OurThenPin = FindPin(UEdGraphSchema_K2::PN_Then);
    UEdGraphPin* OurReturnPin = FindPin(UEdGraphSchema_K2::PN_ReturnValue);

    if (!SpaceConnectionPin || !ObjectClassPin || !DataAssetPin || !OurExecutePin || !OurThenPin || !OurReturnPin)
    {
        CompilerContext.MessageLog.Error(*FString::Printf(TEXT("Missing required pins on @@")), this);
        return;
    }

    // Get ExposeOnSpawn properties to build the array
    UClass* ObjectClass = GetObjectClassFromPin();
    TArray<FCavrnusExposeOnSpawnProperty> ExposeProps;
    if (ObjectClass)
    {
        ExposeProps = FCavrnusSpawnPropertyHelpers::GetExposeOnSpawnProperties(ObjectClass);
    }

    // Build array of FCavrnusSpawnPropertyValue from ExposeOnSpawn pins using helper
    TArray<UK2Node_MakeStruct*> MakeStructNodes;
    UK2Node_MakeArray* MakeArrayNode = nullptr;
    FCavrnusNodeHelpers::BuildExposeOnSpawnPropertyArray(
        CompilerContext,
        SourceGraph,
        this,
        ExposeProps,
        MakeStructNodes,
        MakeArrayNode
    );
    
    // Create function call node for CavrnusConstructObjectFromClassWithArray
    UK2Node_CallFunction* FunctionCallNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
    FunctionCallNode->FunctionReference.SetExternalMember(
        TEXT("CavrnusConstructObjectFromClassWithArray"),
        USpawnedObjectsManager::StaticClass()
    );
    FunctionCallNode->AllocateDefaultPins();

    // Build parameter pin mappings
    TMap<FString, UEdGraphPin*> ParameterPinMappings;
    ParameterPinMappings.Add(TEXT("SpaceConnection"), SpaceConnectionPin);
    ParameterPinMappings.Add(TEXT("ObjectClass"), ObjectClassPin);
    UEdGraphPin* OuterPin = FindPin(TEXT("Outer"));
    if (OuterPin)
    {
        ParameterPinMappings.Add(TEXT("Outer"), OuterPin);
    }
    ParameterPinMappings.Add(TEXT("DataAsset"), DataAssetPin);

    // Wire the function call node using helper
    FCavrnusNodeHelpers::WireFunctionCallNode(
        CompilerContext,
        FunctionCallNode,
        ParameterPinMappings,
        OurExecutePin,
        OurThenPin,
        OurReturnPin
    );

    // Wire ExposeOnSpawn array pin manually (needs special handling)
    UEdGraphPin* FunctionExposeOnSpawnArrayPin = FunctionCallNode->FindPin(TEXT("ExposeOnSpawnValuesArray"));
    if (FunctionExposeOnSpawnArrayPin && MakeArrayNode)
    {
        // Find the output pin from the MakeArray node
        UEdGraphPin* ArrayOutputPin = nullptr;
        for (UEdGraphPin* Pin : MakeArrayNode->Pins)
        {
            if (Pin && Pin->Direction == EGPD_Output)
            {
                ArrayOutputPin = Pin;
                break;
            }
        }
        
        if (ArrayOutputPin)
        {
            ArrayOutputPin->MakeLinkTo(FunctionExposeOnSpawnArrayPin);
        }
    }

    // Break all node links to prevent further expansion
    BreakAllNodeLinks();
}

FText UK2Node_CavrnusConstructObjectFromClass::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    return FText::FromString(TEXT("Cavrnus Construct Object From Class"));
}

FText UK2Node_CavrnusConstructObjectFromClass::GetTooltipText() const
{
    return FText::FromString(TEXT("Constructs a UObject instance of the specified class in the Cavrnus space. ExposeOnSpawn properties are automatically exposed as pins."));
}

UClass* UK2Node_CavrnusConstructObjectFromClass::GetObjectClassFromPin() const
{
    UEdGraphPin* ObjectClassPin = FindPin(TEXT("ObjectClass"));
    if (!ObjectClassPin)
    {
        return nullptr;
    }

    // If pin is connected, we can't determine the class at compile time
    if (ObjectClassPin->LinkedTo.Num() > 0)
    {
        return nullptr;
    }

    // Try to get from default value
    if (ObjectClassPin->DefaultObject)
    {
        if (UClass* Class = Cast<UClass>(ObjectClassPin->DefaultObject))
        {
            return Class;
        }
    }

    return nullptr;
}

void UK2Node_CavrnusConstructObjectFromClass::CreateExposeOnSpawnPins()
{
    UClass* ObjectClass = GetObjectClassFromPin();
    
    // Find where to insert ExposeOnSpawn pins - they should appear immediately after ObjectClass
    // Function signature order: SpaceConnection, ObjectClass, ExposeOnSpawn, Outer, DataAsset
    UEdGraphPin* ObjectClassPin = FindPin(TEXT("ObjectClass"));
    int32 InsertIndex = INDEX_NONE;
    
    if (ObjectClassPin)
    {
        InsertIndex = Pins.IndexOfByKey(ObjectClassPin);
        if (InsertIndex != INDEX_NONE)
        {
            InsertIndex++; // Insert after ObjectClass
        }
    }

    FCavrnusNodeHelpers::CreateExposeOnSpawnPins(
        this,
        ObjectClass,
        CachedObjectClass,
        CachedExposeOnSpawnProperties,
        [this]() { return GetObjectClassFromPin(); },
        [this](FProperty* Prop) { return GetPinTypeForProperty(Prop); },
        InsertIndex
    );

    // Create Outer pin only if it doesn't already exist
    // This prevents duplicate Outer pins when the class changes
    UEdGraphPin* ExistingOuterPin = FindPin(TEXT("Outer"));
    if (!ExistingOuterPin)
    {
        // Get the target position before creating the pin (so it's after all existing pins)
        int32 OuterInsertIndex = Pins.Num();
        
        FEdGraphPinType OuterPinType;
        OuterPinType.PinCategory = UEdGraphSchema_K2::PC_Object;
        OuterPinType.PinSubCategoryObject = UObject::StaticClass();
        UEdGraphPin* OuterPin = CreatePin(EGPD_Input, OuterPinType, TEXT("Outer"));
        if (OuterPin)
        {
            OuterPin->PinFriendlyName = FText::FromString(TEXT("Outer"));
            OuterPin->PinToolTip = TEXT("Optional outer object for the constructed object. Controls object lifetime and garbage collection.");
            
            // Position Outer pin at the very end (after all other pins including ExposeOnSpawn)
            TArray<UEdGraphPin*> OuterPinArray;
            OuterPinArray.Add(OuterPin);
            FCavrnusNodeHelpers::MovePinsToPosition(this, OuterPinArray, OuterInsertIndex);
        }
    }
}

void UK2Node_CavrnusConstructObjectFromClass::RemoveExposeOnSpawnPins()
{
    UClass* CurrentClass = GetObjectClassFromPin();
    FCavrnusNodeHelpers::RemoveExposeOnSpawnPins(
        this,
        CurrentClass,
        CachedExposeOnSpawnProperties,
        [this]() { return GetObjectClassFromPin(); }
    );
}

bool UK2Node_CavrnusConstructObjectFromClass::DoExposeOnSpawnPinsExist() const
{
    // Get the current ObjectClass
    UClass* ObjectClass = GetObjectClassFromPin();
    if (!ObjectClass)
    {
        return false;
    }
    
    // Use cached properties if available to avoid property reflection during compilation
    TArray<FCavrnusExposeOnSpawnProperty> CurrentExposeProperties;
    if (ObjectClass == CachedObjectClass && CachedExposeOnSpawnProperties.Num() > 0)
    {
        // Use cached properties
        CurrentExposeProperties = CachedExposeOnSpawnProperties;
    }
    else if (ObjectClass && ObjectClass->GetFName() != NAME_None)
    {
        // Class changed or no cache, try to get properties (may fail during compilation)
        CurrentExposeProperties = FCavrnusSpawnPropertyHelpers::GetExposeOnSpawnProperties(ObjectClass);
    }
    else
    {
        // Class not ready, use cached properties if available
        if (CachedExposeOnSpawnProperties.Num() > 0)
        {
            CurrentExposeProperties = CachedExposeOnSpawnProperties;
        }
        else
        {
            // No cache and class not ready, assume pins don't exist
            return false;
        }
    }
    
    if (CurrentExposeProperties.Num() == 0)
    {
        // No ExposeOnSpawn properties expected, so pins "exist" (none needed)
        return true;
    }
    
    // Check if all required ExposeOnSpawn pins exist
    for (const FCavrnusExposeOnSpawnProperty& ExposeProp : CurrentExposeProperties)
    {
        UEdGraphPin* Pin = FindPin(FName(*ExposeProp.PropertyName));
        if (!Pin)
        {
            return false;
        }
    }
    
    return true;
}

FEdGraphPinType UK2Node_CavrnusConstructObjectFromClass::GetPinTypeForProperty(FProperty* Prop) const
{
    return FCavrnusNodeHelpers::GetPinTypeForProperty(Prop);
}

TArray<UClass*> UK2Node_CavrnusConstructObjectFromClass::GetValidObjectClassesFromDataAsset() const
{
    TArray<UClass*> ValidClasses;

    // Get DataAsset pin
    UEdGraphPin* DataAssetPin = FindPin(TEXT("DataAsset"));
    UCavrnusSpawnableRegistryDataAsset* DataAsset = nullptr;

    // Check if DataAsset pin has a default object set (not connected)
    if (DataAssetPin && DataAssetPin->LinkedTo.Num() == 0)
    {
        if (DataAssetPin->DefaultObject)
        {
            DataAsset = Cast<UCavrnusSpawnableRegistryDataAsset>(DataAssetPin->DefaultObject);
        }
    }

    // If no DataAsset from pin, try to get default from DataAssetManager
    if (!DataAsset)
    {
        UCavrnusSubsystem* Sub = UCavrnusSubsystem::Get();
        UCavrnusDataAssetManager* DataAssetManager = (Sub && Sub->EditorContext)
            ? Sub->EditorContext->Get<UCavrnusDataAssetManager>() : nullptr;
        if (DataAssetManager)
        {
            DataAsset = DataAssetManager->GetAsset<UCavrnusSpawnableRegistryDataAsset>();
        }
    }

    if (!DataAsset)
    {
        return ValidClasses;
    }

    // Collect valid object classes from DataAsset entries
    // Matching Unreal Engine's ConstructObject - only UObject classes (excluding AActor)
    for (const FCavrnusSpawnableEntry& Entry : DataAsset->GetEntries())
    {
        // Only check ObjectClass entries (matching Unreal Engine's ConstructObject behavior)
        // ObjectInstance entries are not used by ConstructObject
        if (!Entry.ObjectClass.IsNull())
        {
            // Try to get the class - first check if already loaded, otherwise load it
            UClass* LoadedClass = nullptr;
            FSoftObjectPath ClassPath = Entry.ObjectClass.ToSoftObjectPath();
            if (ClassPath.IsValid())
            {
                if (Entry.ObjectClass.IsValid())
                {
                    LoadedClass = Entry.ObjectClass.Get();
                }
                else
                {
                    LoadedClass = Entry.ObjectClass.LoadSynchronous();
                }
                // Only include UObject classes that are NOT AActor (matching Unreal Engine's ConstructObject)
                if (LoadedClass && 
                    LoadedClass->IsChildOf(UObject::StaticClass()) && 
                    !LoadedClass->IsChildOf(AActor::StaticClass()))
                {
                    ValidClasses.AddUnique(LoadedClass);
                }
            }
        }
    }

    return ValidClasses;
}

TArray<UClass*> UK2Node_CavrnusConstructObjectFromClass::GetValidObjectClasses() const
{
    return GetValidObjectClassesFromDataAsset();
}

void UK2Node_CavrnusConstructObjectFromClass::UpdateObjectClassPinFiltering()
{
    // Update the ObjectClass pin to filter by valid classes from DataAsset
    // This is similar to how K2Node_CavrnusSpawnActorFromClass filters ActorClass
    UEdGraphPin* ObjectClassPin = FindPin(TEXT("ObjectClass"));
    if (!ObjectClassPin)
    {
        return;
    }

    // Get valid classes from the currently selected DataAsset
    CachedValidClasses = GetValidObjectClassesFromDataAsset();

    // Build a tooltip string with the allowed classes
    FString ClassNames;
    for (int32 i = 0; i < FMath::Min(CachedValidClasses.Num(), 5); ++i)
    {
        if (i > 0) ClassNames += TEXT(", ");
        ClassNames += CachedValidClasses[i]->GetName();
    }
    if (CachedValidClasses.Num() > 5)
    {
        ClassNames += FString::Printf(TEXT(", ... (%d total)"), CachedValidClasses.Num());
    }

    ObjectClassPin->PinToolTip = FString::Printf(
        TEXT("Object class to construct. Must be one of the classes registered in the selected CavrnusSpawnableRegistryDataAsset.\n\nValid classes: %s"),
        *ClassNames
    );

    // Store the valid classes in pin metadata (for fallback tooltip parsing)
    FString ValidClassesString;
    for (UClass* ValidClass : CachedValidClasses)
    {
        if (!ValidClass || !IsValid(ValidClass)) continue;
        
        FString ClassPath = ValidClass->GetPathName();
        
        // Filter out garbage paths (stale classes have paths like "Class None.:/Game/...")
        if (ClassPath.IsEmpty() || ClassPath.StartsWith(TEXT("Class None.")))
        {
            UE_LOG(LogCavrnusBlueprintModule, Warning, TEXT("[UpdateObjectClassPinFiltering] Filtered out invalid path: '%s'"), *ClassPath);
            continue; // Skip invalid/stale paths
        }
        
        if (!ValidClassesString.IsEmpty()) ValidClassesString += TEXT(",");
        ValidClassesString += ClassPath;
    }

    ObjectClassPin->PinToolTip += FString::Printf(TEXT("\n\nValidClasses:%s"), *ValidClassesString);
}

