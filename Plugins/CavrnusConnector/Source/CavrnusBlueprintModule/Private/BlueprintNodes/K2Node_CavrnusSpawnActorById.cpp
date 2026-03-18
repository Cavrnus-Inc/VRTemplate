// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "BlueprintNodes/K2Node_CavrnusSpawnActorById.h"
#include "BlueprintNodes/CavrnusNodeHelpers.h"
#include "Managers/SpawnedObjects/SpawnedObjectsManager.h"
#include "Managers/SpawnedObjects/CavrnusSpawnPropertyHelpers.h"
#include "AssetManager/DataAssets/CavrnusSpawnableRegistryDataAsset.h"
#include "AssetManager/CavrnusDataAssetManager.h"
#include "Core/Subsystems/CavrnusSubsystem.h"
#include "Core/Contexts/CavrnusEditorContext.h"
#include "CavrnusFunctionLibrary.h"
#include "CavrnusPropertiesContainer.h"
#include "KismetCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraphSchema_K2.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "UObject/UnrealType.h"
#include "Engine/Engine.h"
#include "Engine/EngineTypes.h"
#include "K2Node_CallFunction.h"
#include "K2Node_VariableGet.h"
#include "KismetCompiler.h"
#include "CavrnusBlueprintModule.h"
#include "Managers/SpawnedObjects/CavrnusSpawnPropertyValue.h"
#include "K2Node_MakeStruct.h"
#include "K2Node_MakeArray.h"

UK2Node_CavrnusSpawnActorById::UK2Node_CavrnusSpawnActorById(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , CachedActorClass(nullptr)
{
}

void UK2Node_CavrnusSpawnActorById::AllocateDefaultPins()
{
    // Create execution pins
    CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
    CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);

    // Create return value pin (AActor*)
    FEdGraphPinType ReturnPinType;
    ReturnPinType.PinCategory = UEdGraphSchema_K2::PC_Object;
    ReturnPinType.PinSubCategoryObject = AActor::StaticClass();
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
        SpaceConnectionPin->PinToolTip = TEXT("The space connection where the object will be spawned");
    }

    // DataAsset pin
    FEdGraphPinType DataAssetPinType;
    DataAssetPinType.PinCategory = UEdGraphSchema_K2::PC_Object;
    DataAssetPinType.PinSubCategoryObject = UCavrnusSpawnableRegistryDataAsset::StaticClass();
    UEdGraphPin* DataAssetPin = CreatePin(EGPD_Input, DataAssetPinType, TEXT("DataAsset"));
    if (DataAssetPin)
    {
        DataAssetPin->PinFriendlyName = FText::FromString(TEXT("Data Asset"));
        DataAssetPin->PinToolTip = TEXT("Optional DataAsset to use for lookup (defaults to main one)");
    }

    // WellKnownObjectId pin
    FEdGraphPinType WellKnownObjectIdPinType;
    WellKnownObjectIdPinType.PinCategory = UEdGraphSchema_K2::PC_String;
    UEdGraphPin* WellKnownObjectIdPin = CreatePin(EGPD_Input, WellKnownObjectIdPinType, TEXT("WellKnownObjectId"));
    if (WellKnownObjectIdPin)
    {
        WellKnownObjectIdPin->PinFriendlyName = FText::FromString(TEXT("Well Known Object Id"));
        WellKnownObjectIdPin->PinToolTip = TEXT("Object ID from DataAsset (filtered by selected DataAsset)");
    }

    // SpawnTransform pin
    FEdGraphPinType TransformPinType;
    TransformPinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
    TransformPinType.PinSubCategoryObject = TBaseStructure<FTransform>::Get();
    UEdGraphPin* TransformPin = CreatePin(EGPD_Input, TransformPinType, TEXT("SpawnTransform"));
    if (TransformPin)
    {
        TransformPin->PinFriendlyName = FText::FromString(TEXT("Spawn Transform"));
        TransformPin->PinToolTip = TEXT("Initial transform for the spawned actor");
        FTransform DefaultTransform = FTransform::Identity;
        TransformPin->DefaultValue = DefaultTransform.ToString();
    }

    // CollisionHandling pin
    FEdGraphPinType CollisionHandlingPinType;
    CollisionHandlingPinType.PinCategory = UEdGraphSchema_K2::PC_Byte;
    CollisionHandlingPinType.PinSubCategoryObject = StaticEnum<ESpawnActorCollisionHandlingMethod>();
    UEdGraphPin* CollisionHandlingPin = CreatePin(EGPD_Input, CollisionHandlingPinType, TEXT("CollisionHandlingOverride"));
    if (CollisionHandlingPin)
    {
        CollisionHandlingPin->PinFriendlyName = FText::FromString(TEXT("Collision Handling Override"));
        CollisionHandlingPin->PinToolTip = TEXT("Collision handling override");
        const UEnum* MethodEnum = StaticEnum<ESpawnActorCollisionHandlingMethod>();
        CollisionHandlingPin->DefaultValue = MethodEnum->GetNameStringByValue(static_cast<int64>(ESpawnActorCollisionHandlingMethod::AlwaysSpawn));
    }

    // Populate valid IDs based on current DataAsset selection
    UpdateIdPinFiltering();

    // Cache actor class and create ExposeOnSpawn pins based on it
    // Only update CachedActorClass if we get a valid class - preserve existing cached value during refresh
    UClass* ActorClassFromId = GetActorClassFromId();
    if (ActorClassFromId)
    {
        CachedActorClass = ActorClassFromId;
    }
    // If GetActorClassFromId() returns nullptr, preserve existing CachedActorClass (if any)
    // This allows CreateExposeOnSpawnPins() to use cached properties during node refresh
    
    CreateExposeOnSpawnPins();
    
    // Ensure CachedExposeOnSpawnProperties is populated
    if (CachedExposeOnSpawnProperties.Num() == 0 && CachedActorClass)
    {
        CachedExposeOnSpawnProperties = FCavrnusSpawnPropertyHelpers::GetExposeOnSpawnProperties(CachedActorClass);
    }
}

void UK2Node_CavrnusSpawnActorById::NotifyPinConnectionListChanged(UEdGraphPin* Pin)
{

    // If WellKnownObjectId pin changed, update ExposeOnSpawn pins
    if (Pin && Pin->PinName == TEXT("WellKnownObjectId"))
    {
        RemoveExposeOnSpawnPins();
        CreateExposeOnSpawnPins();
        CachedActorClass = GetActorClassFromId();
    }
    // If DataAsset pin changed, update ID pin filtering and clear WellKnownObjectId
    else if (Pin && Pin->PinName == TEXT("DataAsset"))
    {
        // Clear the WellKnownObjectId pin since the valid IDs list has changed
        UEdGraphPin* WellKnownObjectIdPin = FindPin(TEXT("WellKnownObjectId"));
        if (WellKnownObjectIdPin)
        {
            WellKnownObjectIdPin->DefaultValue = TEXT("");
            
            // Remove ExposeOnSpawn pins since we no longer have a valid ID
            RemoveExposeOnSpawnPins();
            CachedActorClass = nullptr;
            
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
        
        UpdateIdPinFiltering();
    }
}

void UK2Node_CavrnusSpawnActorById::PostReconstructNode()
{

    // Update ID pin filtering based on current DataAsset
    UpdateIdPinFiltering();

    // Check if actor class has changed (default value, not just connection)
    UClass* CurrentActorClass = GetActorClassFromId();
    
    // Only update pins if we have a valid class AND it's different from cached
    // If CurrentActorClass is nullptr, preserve existing pins (they might be from a valid class that just isn't loaded yet)
    if (CurrentActorClass && CurrentActorClass != CachedActorClass)
    {
        RemoveExposeOnSpawnPins();
        CreateExposeOnSpawnPins();
        CachedActorClass = CurrentActorClass;
    }
    // If we have a cached class but can't determine current class, ensure pins exist
    else if (CachedActorClass && !CurrentActorClass)
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
    else if (!CurrentActorClass && !CachedActorClass)
    {
        RemoveExposeOnSpawnPins();
    }
}

void UK2Node_CavrnusSpawnActorById::PinDefaultValueChanged(UEdGraphPin* Pin)
{

    if (!Pin)
    {
        return;
    }

    // If WellKnownObjectId pin default value changed, update ExposeOnSpawn pins
    if (Pin->PinName == TEXT("WellKnownObjectId"))
    {
        UClass* CurrentActorClass = GetActorClassFromId();
        
        if (CurrentActorClass == nullptr)
        {
            CachedActorClass = nullptr;
            RemoveExposeOnSpawnPins();
            // Also call CreateExposeOnSpawnPins() to handle Instigator pin removal
            CreateExposeOnSpawnPins();
        }
        else if (CurrentActorClass != CachedActorClass)
        {
            RemoveExposeOnSpawnPins();
            CreateExposeOnSpawnPins();
            CachedActorClass = CurrentActorClass;
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
    // If DataAsset pin changed, clear WellKnownObjectId and update filtering
    else if (Pin->PinName == TEXT("DataAsset"))
    {
        // Clear the WellKnownObjectId pin since the valid IDs list has changed
        UEdGraphPin* WellKnownObjectIdPin = FindPin(TEXT("WellKnownObjectId"));
        if (WellKnownObjectIdPin)
        {
            WellKnownObjectIdPin->DefaultValue = TEXT("");
            
            // Remove ExposeOnSpawn pins since we no longer have a valid ID
            RemoveExposeOnSpawnPins();
            CachedActorClass = nullptr;
            
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
        
        UpdateIdPinFiltering();
    }
}

void UK2Node_CavrnusSpawnActorById::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
    // Get input pins
    UEdGraphPin* SpaceConnectionPin = FindPin(TEXT("SpaceConnection"));
    UEdGraphPin* WellKnownObjectIdPin = FindPin(TEXT("WellKnownObjectId"));
    UEdGraphPin* TransformPin = FindPin(TEXT("SpawnTransform"));
    UEdGraphPin* CollisionHandlingPin = FindPin(TEXT("CollisionHandlingOverride"));
    UEdGraphPin* DataAssetPin = FindPin(TEXT("DataAsset"));
    UEdGraphPin* OurExecutePin = FindPin(UEdGraphSchema_K2::PN_Execute);
    UEdGraphPin* OurThenPin = FindPin(UEdGraphSchema_K2::PN_Then);
    UEdGraphPin* OurReturnPin = FindPin(UEdGraphSchema_K2::PN_ReturnValue);

    if (!SpaceConnectionPin || !WellKnownObjectIdPin || !TransformPin || !CollisionHandlingPin || !DataAssetPin || !OurExecutePin || !OurThenPin || !OurReturnPin)
    {
        CompilerContext.MessageLog.Error(*FString::Printf(TEXT("Missing required pins on @@")), this);
        return;
    }

    // Get ExposeOnSpawn properties to build the array
    UClass* ActorClass = GetActorClassFromId();
    TArray<FCavrnusExposeOnSpawnProperty> ExposeProps;
    if (ActorClass)
    {
        ExposeProps = FCavrnusSpawnPropertyHelpers::GetExposeOnSpawnProperties(ActorClass);
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
    
    // Create function call node for CavrnusSpawnActorByIdWithArray
    UK2Node_CallFunction* FunctionCallNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
    FunctionCallNode->FunctionReference.SetExternalMember(
        TEXT("CavrnusSpawnActorByIdWithArray"),
        USpawnedObjectsManager::StaticClass()
    );
    FunctionCallNode->AllocateDefaultPins();

    // Build parameter pin mappings
    TMap<FString, UEdGraphPin*> ParameterPinMappings;
    ParameterPinMappings.Add(TEXT("SpaceConnection"), SpaceConnectionPin);
    ParameterPinMappings.Add(TEXT("WellKnownObjectId"), WellKnownObjectIdPin);
    ParameterPinMappings.Add(TEXT("SpawnTransform"), TransformPin);
    ParameterPinMappings.Add(TEXT("CollisionHandlingOverride"), CollisionHandlingPin);
    UEdGraphPin* OwnerPin = FindPin(TEXT("Owner"));
    if (OwnerPin)
    {
        ParameterPinMappings.Add(TEXT("Owner"), OwnerPin);
    }
    ParameterPinMappings.Add(TEXT("DataAsset"), DataAssetPin);
    UEdGraphPin* InstigatorPin = FindPin(TEXT("Instigator"));
    if (InstigatorPin)
    {
        // Note: Instigator parameter may not exist in the function signature yet
        // This is prepared for future support
        ParameterPinMappings.Add(TEXT("Instigator"), InstigatorPin);
    }

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

FText UK2Node_CavrnusSpawnActorById::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    return FText::FromString(TEXT("Cavrnus Spawn Actor By Id"));
}

FText UK2Node_CavrnusSpawnActorById::GetTooltipText() const
{
    return FText::FromString(TEXT("Spawns an actor using a well-known object identifier from the DataAsset. ExposeOnSpawn properties are automatically exposed as pins."));
}

void UK2Node_CavrnusSpawnActorById::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    // Get the function we want to override (use the WithArray version which is a UFUNCTION)
    UFunction* FunctionToRegister = USpawnedObjectsManager::StaticClass()->FindFunctionByName(TEXT("CavrnusSpawnActorByIdWithArray"));
    if (!FunctionToRegister)
    {
        UE_LOG(LogCavrnusBlueprintModule, Error, TEXT("[GetMenuActions] CavrnusSpawnActorByIdWithArray function not found!"));
        return;
    }

    // Register for the function (not the class) - this will replace the default UK2Node_CallFunction
    if (ActionRegistrar.IsOpenForRegistration(FunctionToRegister))
    {
        // Create a node spawner for our custom node class
        UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
        check(NodeSpawner != nullptr);

        // Set a custom menu name to distinguish from the default UK2Node_CallFunction entry
        // Use a clear name so users know this is the enhanced version with ExposeOnSpawn support
        NodeSpawner->DefaultMenuSignature.MenuName = FText::FromString(TEXT("Cavrnus Spawn Actor By Id"));
        NodeSpawner->DefaultMenuSignature.Category = FText::FromString(TEXT("Cavrnus|Objects"));
        NodeSpawner->DefaultMenuSignature.Tooltip = FText::FromString(TEXT("Spawns an actor by ID with ExposeOnSpawn property support."));

        // No need to set TargetFunction anymore since we manually create pins
        // The node will call CavrnusSpawnActorByIdWithArray in ExpandNode

        // Register for the function - this should replace the default UK2Node_CallFunction
        ActionRegistrar.AddBlueprintAction(FunctionToRegister, NodeSpawner);
    }
}

void UK2Node_CavrnusSpawnActorById::UpdateIdPinFiltering()
{
    // This method is called to update the WellKnownObjectId pin's metadata/tooltip
    // The actual filtering is done by the custom pin widget (SCavrnusSpawnActorIdPin)
    // which calls GetValidIds() to get the filtered list
}

TArray<FName> UK2Node_CavrnusSpawnActorById::GetValidIds() const
{
    return GetValidIdsFromDataAsset();
}

TArray<FName> UK2Node_CavrnusSpawnActorById::GetValidIdsFromDataAsset() const
{
    TArray<FName> ValidIds;

    UEdGraphPin* DataAssetPin = FindPin(TEXT("DataAsset"));
    UCavrnusSpawnableRegistryDataAsset* DataAsset = nullptr;

    if (DataAssetPin && DataAssetPin->LinkedTo.Num() == 0 && DataAssetPin->DefaultObject)
    {
        DataAsset = Cast<UCavrnusSpawnableRegistryDataAsset>(DataAssetPin->DefaultObject);
    }

    if (!DataAsset)
    {
        // Try to get default DataAsset from DataAssetManager
        UCavrnusSubsystem* Sub = UCavrnusSubsystem::Get();
        UCavrnusDataAssetManager* DataAssetManager = (Sub && Sub->EditorContext)
            ? Sub->EditorContext->Get<UCavrnusDataAssetManager>() : nullptr;
        if (DataAssetManager)
        {
            DataAsset = DataAssetManager->GetAsset<UCavrnusSpawnableRegistryDataAsset>();
        }
    }

    if (DataAsset)
    {
        // Collect valid IDs from DataAsset entries - only include entries with valid ActorClass
        // This matches the behavior of GetValidActorClassesFromDataAsset() in K2Node_CavrnusSpawnActorFromClass
        for (const FCavrnusSpawnableEntry& Entry : DataAsset->GetEntries())
        {
            if (Entry.ActorClass.IsNull())
            {
                continue;  // Skip entries without ActorClass
            }

            // Try to get the class - first check if already loaded, otherwise load it
            UClass* LoadedClass = nullptr;
            FSoftObjectPath ClassPath = Entry.ActorClass.ToSoftObjectPath();
            if (ClassPath.IsValid())
            {
                if (Entry.ActorClass.IsValid())
                {
                    LoadedClass = Entry.ActorClass.Get();
                }
                else
                {
                    LoadedClass = Entry.ActorClass.LoadSynchronous();
                }
            }

            // Only add if we successfully loaded a valid actor class
            if (LoadedClass && LoadedClass->IsChildOf(AActor::StaticClass()))
            {
                if (!Entry.Key.IsNone())
                {
                    ValidIds.AddUnique(Entry.Key);
                }
            }
        }
    }

    return ValidIds;
}

UClass* UK2Node_CavrnusSpawnActorById::GetActorClassFromId() const
{
    UEdGraphPin* WellKnownObjectIdPin = FindPin(TEXT("WellKnownObjectId"));
    UEdGraphPin* DataAssetPin = FindPin(TEXT("DataAsset"));
    
    if (!WellKnownObjectIdPin)
    {
        return nullptr;
    }

    // Get the WellKnownObjectId value
    FString WellKnownObjectId;
    if (WellKnownObjectIdPin->LinkedTo.Num() > 0)
    {
        // Pin is connected, we can't determine the ID at compile time
        return nullptr;
    }
    else
    {
        WellKnownObjectId = WellKnownObjectIdPin->DefaultValue;
    }

    if (WellKnownObjectId.IsEmpty())
    {
        return nullptr;
    }

    // Get the DataAsset
    UCavrnusSpawnableRegistryDataAsset* DataAsset = nullptr;
    if (DataAssetPin && DataAssetPin->DefaultObject)
    {
        DataAsset = Cast<UCavrnusSpawnableRegistryDataAsset>(DataAssetPin->DefaultObject);
    }

    if (!DataAsset)
    {
        // Try to get default DataAsset from DataAssetManager
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
        return nullptr;
    }

    // Look up the actor class from the DataAsset (matching Unreal Engine's SpawnActor - only AActor classes)
    TSubclassOf<AActor> ActorClass = nullptr;
    
    FName KeyName(*WellKnownObjectId);
    
    // Only look for actor class - matching Unreal Engine's SpawnActor behavior
    TOptional<TSoftClassPtr<AActor>> ActorClassOpt = DataAsset->GetActorClassForKey(KeyName);
    if (ActorClassOpt.IsSet())
    {
        TSoftClassPtr<AActor> SoftClass = ActorClassOpt.GetValue();
        UClass* LoadedClass = SoftClass.LoadSynchronous();
        if (LoadedClass && LoadedClass->IsChildOf(AActor::StaticClass()))
        {
            ActorClass = LoadedClass;
        }
    }
    
    return ActorClass;
}

void UK2Node_CavrnusSpawnActorById::CreateExposeOnSpawnPins()
{
    UClass* ActorClass = GetActorClassFromId();
    
    // Find where to insert Owner pin - it should appear immediately after CollisionHandling
    // Function signature order: SpaceConnection, DataAsset, WellKnownObjectId, Transform, CollisionHandling, Owner, ExposeOnSpawn, Instigator
    UEdGraphPin* CollisionHandlingPin = FindPin(TEXT("CollisionHandlingOverride"));
    int32 OwnerInsertIndex = INDEX_NONE;
    
    if (CollisionHandlingPin)
    {
        OwnerInsertIndex = Pins.IndexOfByKey(CollisionHandlingPin);
        if (OwnerInsertIndex != INDEX_NONE)
        {
            OwnerInsertIndex++; // Insert after CollisionHandlingOverride
        }
    }

    // Create Owner pin only if it doesn't already exist
    // This prevents duplicate Owner pins when the class changes
    UEdGraphPin* ExistingOwnerPin = FindPin(TEXT("Owner"));
    if (!ExistingOwnerPin)
    {
        FEdGraphPinType OwnerPinType;
        OwnerPinType.PinCategory = UEdGraphSchema_K2::PC_Object;
        OwnerPinType.PinSubCategoryObject = AActor::StaticClass();
        UEdGraphPin* OwnerPin = CreatePin(EGPD_Input, OwnerPinType, TEXT("Owner"));
        if (OwnerPin)
        {
            OwnerPin->PinFriendlyName = FText::FromString(TEXT("Owner"));
            OwnerPin->PinToolTip = TEXT("Optional owner actor for the spawned actor. When the owner is destroyed, owned actors are also destroyed.");
            OwnerPin->bAdvancedView = true; // Mark as Advanced (hidden by default)
            
            // Position Owner pin right after CollisionHandlingOverride
            TArray<UEdGraphPin*> OwnerPinArray;
            OwnerPinArray.Add(OwnerPin);
            FCavrnusNodeHelpers::MovePinsToPosition(this, OwnerPinArray, OwnerInsertIndex);
        }
    }
    else
    {
        // Ensure existing Owner pin is marked as Advanced
        ExistingOwnerPin->bAdvancedView = true;
    }

    // Find where to insert ExposeOnSpawn pins - they should appear after Owner
    int32 ExposeOnSpawnInsertIndex = INDEX_NONE;
    UEdGraphPin* OwnerPinForInsert = FindPin(TEXT("Owner"));
    if (OwnerPinForInsert)
    {
        ExposeOnSpawnInsertIndex = Pins.IndexOfByKey(OwnerPinForInsert);
        if (ExposeOnSpawnInsertIndex != INDEX_NONE)
        {
            ExposeOnSpawnInsertIndex++; // Insert after Owner
        }
    }
    else if (CollisionHandlingPin)
    {
        // Fallback: if Owner pin doesn't exist, use CollisionHandling position
        ExposeOnSpawnInsertIndex = Pins.IndexOfByKey(CollisionHandlingPin);
        if (ExposeOnSpawnInsertIndex != INDEX_NONE)
        {
            ExposeOnSpawnInsertIndex++; // Insert after CollisionHandlingOverride
        }
    }

    FCavrnusNodeHelpers::CreateExposeOnSpawnPins(
        this,
        ActorClass,
        CachedActorClass,
        CachedExposeOnSpawnProperties,
        [this]() { return GetActorClassFromId(); },
        [this](FProperty* Prop) { return GetPinTypeForProperty(Prop); },
        ExposeOnSpawnInsertIndex
    );

    // Create Instigator pin only if it doesn't already exist and the class is AActor or child of AActor
    // This prevents duplicate Instigator pins when the class changes
    UEdGraphPin* ExistingInstigatorPin = FindPin(TEXT("Instigator"));
    
    // If ActorClass is explicitly nullptr (None selected), always remove Instigator pin
    // Only use cached class during refresh scenarios when ActorClass might be temporarily unavailable
    bool bShouldCreateInstigator = false;
    if (!ExistingInstigatorPin)
    {
        if (ActorClass)
        {
            // Only create Instigator pin for AActor classes
            bool bIsActorClass = ActorClass->IsChildOf(AActor::StaticClass()) || ActorClass == AActor::StaticClass();
            bShouldCreateInstigator = bIsActorClass;
            // UE_LOG(LogCavrnusBlueprintModule, Log, TEXT("[CreateExposeOnSpawnPins] ActorClass is valid, bIsActorClass: %d, bShouldCreateInstigator: %d"),
            //    bIsActorClass ? 1 : 0, bShouldCreateInstigator ? 1 : 0);
        }
        // During refresh, if ActorClass is nullptr but we have a valid cached class, check it
        else if (CachedActorClass)
        {
            // Only create if cached class is an AActor (refresh scenario)
            bool bIsActorClass = CachedActorClass->IsChildOf(AActor::StaticClass()) || CachedActorClass == AActor::StaticClass();
            bShouldCreateInstigator = bIsActorClass;
            // UE_LOG(LogCavrnusBlueprintModule, Log, TEXT("[CreateExposeOnSpawnPins] Using CachedActorClass, bIsActorClass: %d, bShouldCreateInstigator: %d"),
            //    bIsActorClass ? 1 : 0, bShouldCreateInstigator ? 1 : 0);
        }
        else
        {
            // UE_LOG(LogCavrnusBlueprintModule, Log, TEXT("[CreateExposeOnSpawnPins] Both ActorClass and CachedActorClass are nullptr, will not create Instigator"));
        }
    }

    if (bShouldCreateInstigator)
    {
        // Get the target position at the very end (after all existing pins)
        int32 InstigatorInsertIndex = Pins.Num();
        
        FEdGraphPinType InstigatorPinType;
        InstigatorPinType.PinCategory = UEdGraphSchema_K2::PC_Object;
        InstigatorPinType.PinSubCategoryObject = AActor::StaticClass();
        UEdGraphPin* InstigatorPin = CreatePin(EGPD_Input, InstigatorPinType, TEXT("Instigator"));
        if (InstigatorPin)
        {
            InstigatorPin->PinFriendlyName = FText::FromString(TEXT("Instigator"));
            InstigatorPin->PinToolTip = TEXT("Optional instigator actor for the spawned actor. Used for damage attribution and other gameplay systems.");
            
            // Position Instigator pin at the very end (after all other pins including ExposeOnSpawn)
            TArray<UEdGraphPin*> InstigatorPinArray;
            InstigatorPinArray.Add(InstigatorPin);
            FCavrnusNodeHelpers::MovePinsToPosition(this, InstigatorPinArray, InstigatorInsertIndex);
        }
    }
    else if (ExistingInstigatorPin)
    {
        // Remove Instigator pin if:
        // 1. ActorClass is explicitly nullptr (None selected) - always remove
        // 2. ActorClass exists but is not an AActor - remove
        // 3. Both ActorClass and CachedActorClass are nullptr - remove
        // Only keep if ActorClass is an AActor, or if ActorClass is nullptr but CachedActorClass is a valid AActor (refresh scenario)
        bool bShouldRemove = true;
        
        if (ActorClass)
        {
            // Check if current class is an AActor
            bool bIsActorClass = ActorClass->IsChildOf(AActor::StaticClass()) || ActorClass == AActor::StaticClass();
            if (bIsActorClass)
            {
                bShouldRemove = false; // Keep the pin if current class is an AActor
            }
            UE_LOG(LogCavrnusBlueprintModule, Log, TEXT("[CreateExposeOnSpawnPins] ExistingInstigatorPin: ActorClass is valid, bIsActorClass: %d, bShouldRemove: %d"),
                bIsActorClass ? 1 : 0, bShouldRemove ? 1 : 0);
        }
        else if (CachedActorClass)
        {
            // During refresh, if ActorClass is nullptr but cached is valid, check cached
            bool bIsActorClass = CachedActorClass->IsChildOf(AActor::StaticClass()) || CachedActorClass == AActor::StaticClass();
            if (bIsActorClass)
            {
                bShouldRemove = false; // Keep the pin during refresh if cached class is an AActor
            }
            UE_LOG(LogCavrnusBlueprintModule, Log, TEXT("[CreateExposeOnSpawnPins] ExistingInstigatorPin: Using CachedActorClass, bIsActorClass: %d, bShouldRemove: %d"),
                bIsActorClass ? 1 : 0, bShouldRemove ? 1 : 0);
        }
        else
        {
            // UE_LOG(LogCavrnusBlueprintModule, Log, TEXT("[CreateExposeOnSpawnPins] ExistingInstigatorPin: Both are nullptr, bShouldRemove: %d (will remove)"),
            //     bShouldRemove ? 1 : 0);
        }
        
        if (bShouldRemove)
        {
            // UE_LOG(LogCavrnusBlueprintModule, Log, TEXT("[CreateExposeOnSpawnPins] Removing Instigator pin"));
            RemovePin(ExistingInstigatorPin);
        }
        else
        {
            // UE_LOG(LogCavrnusBlueprintModule, Log, TEXT("[CreateExposeOnSpawnPins] Keeping Instigator pin"));
        }
    }
}

void UK2Node_CavrnusSpawnActorById::RemoveExposeOnSpawnPins()
{
    UClass* CurrentClass = GetActorClassFromId();
    FCavrnusNodeHelpers::RemoveExposeOnSpawnPins(
        this,
        CurrentClass,
        CachedExposeOnSpawnProperties,
        [this]() { return GetActorClassFromId(); }
    );
    CachedExposeOnSpawnProperties.Empty();
}

FEdGraphPinType UK2Node_CavrnusSpawnActorById::GetPinTypeForProperty(FProperty* Prop) const
{
    return FCavrnusNodeHelpers::GetPinTypeForProperty(Prop);
}

