// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "BlueprintNodes/K2Node_CavrnusSpawnActorFromClass.h"
#include "BlueprintNodes/CavrnusNodeHelpers.h"
#include "Managers/SpawnedObjects/SpawnedObjectsManager.h"
#include "Managers/SpawnedObjects/CavrnusSpawnPropertyHelpers.h"
#include "Managers/SpawnedObjects/CavrnusSpawnPropertyValue.h"
#include "Types/CavrnusSpaceConnection.h"
#include "AssetManager/DataAssets/CavrnusSpawnableRegistryDataAsset.h"
#include "AssetManager/CavrnusDataAssetManager.h"
#include "Core/Subsystems/CavrnusSubsystem.h"
#include "Core/Contexts/CavrnusEditorContext.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "CavrnusFunctionLibrary.h"
#include "CavrnusPropertiesContainer.h"
#include "CavrnusBlueprintModule.h"
#include "KismetCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraphSchema_K2.h"
#include "UObject/UnrealType.h"
#include "Engine/Engine.h"
#include "K2Node_CallFunction.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "K2Node_Literal.h"
#include "K2Node_MakeStruct.h"
#include "K2Node_MakeArray.h"
#include "EdGraph/EdGraphSchema.h"
#include "KismetCompiler.h"
#include "Kismet2/CompilerResultsLog.h"
#include "BlueprintCompilationManager.h"

UK2Node_CavrnusSpawnActorFromClass::UK2Node_CavrnusSpawnActorFromClass(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , CachedActorClass(nullptr)
{
}

void UK2Node_CavrnusSpawnActorFromClass::AllocateDefaultPins()
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
        DataAssetPin->PinToolTip = TEXT("Optional DataAsset to use for validation (defaults to main one)");
    }

    // ActorClass pin
    FEdGraphPinType ActorClassPinType;
    ActorClassPinType.PinCategory = UEdGraphSchema_K2::PC_Class;
    ActorClassPinType.PinSubCategoryObject = AActor::StaticClass();
    UEdGraphPin* ActorClassPin = CreatePin(EGPD_Input, ActorClassPinType, TEXT("ActorClass"));
    if (ActorClassPin)
    {
        ActorClassPin->PinFriendlyName = FText::FromString(TEXT("Class"));
        ActorClassPin->PinToolTip = TEXT("Actor class to spawn (filtered by DataAsset)");
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

    // Populate valid classes based on current DataAsset selection
    UpdateActorClassPinFiltering();

    // Fix Transform pin default value to avoid parsing warning
    UEdGraphPin* SpawnTransformPin = FindPin(TEXT("SpawnTransform"));
    if (SpawnTransformPin && SpawnTransformPin->DefaultValue.IsEmpty())
    {
        // Set a valid default Transform value (identity transform)
        FTransform DefaultTransform = FTransform::Identity;
        SpawnTransformPin->DefaultValue = DefaultTransform.ToString();
    }

    // Cache class and create ExposeOnSpawn pins based on it
    // Only update CachedActorClass if we get a valid class - preserve existing cached value during refresh
    UClass* ActorClassFromPin = GetActorClassFromPin();
    if (ActorClassFromPin)
    {
        CachedActorClass = ActorClassFromPin;
    }
    // If GetActorClassFromPin() returns nullptr, preserve existing CachedActorClass (if any)
    // This allows CreateExposeOnSpawnPins() to use cached properties during node refresh
    
    CreateExposeOnSpawnPins();
    
    // Ensure CachedExposeOnSpawnProperties is populated
    if (CachedExposeOnSpawnProperties.Num() == 0 && CachedActorClass)
    {
        CachedExposeOnSpawnProperties = FCavrnusSpawnPropertyHelpers::GetExposeOnSpawnProperties(CachedActorClass);
    }
}

void UK2Node_CavrnusSpawnActorFromClass::NotifyPinConnectionListChanged(UEdGraphPin* Pin)
{

    // If ActorClass pin changed, update ExposeOnSpawn pins
    if (Pin && Pin->PinName == TEXT("ActorClass"))
    {
        RemoveExposeOnSpawnPins();
        CreateExposeOnSpawnPins();
        CachedActorClass = GetActorClassFromPin();
    }
    // If DataAsset pin changed, update ActorClass pin filtering and clear ActorClass
    else if (Pin && Pin->PinName == TEXT("DataAsset"))
    {
        // Clear the ActorClass pin since the valid classes list has changed
        UEdGraphPin* ActorClassPin = FindPin(TEXT("ActorClass"));
        if (ActorClassPin)
        {
            ActorClassPin->DefaultObject = nullptr;
            ActorClassPin->DefaultValue = TEXT("");
            
            // Remove ExposeOnSpawn pins since we no longer have a valid actor class
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
        
        UpdateActorClassPinFiltering();
    }
}

void UK2Node_CavrnusSpawnActorFromClass::PostReconstructNode()
{

    // Update ActorClass pin filtering based on current DataAsset
    UpdateActorClassPinFiltering();

    // Check if ActorClass has changed (default value, not just connection)
    UClass* CurrentActorClass = GetActorClassFromPin();
    
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

void UK2Node_CavrnusSpawnActorFromClass::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    // Register this node class directly (not tied to a UFUNCTION since we removed it)
    UClass* NodeClass = GetClass();
    if (ActionRegistrar.IsOpenForRegistration(NodeClass))
    {
        UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(NodeClass);
        check(NodeSpawner != nullptr);

        NodeSpawner->DefaultMenuSignature.MenuName = FText::FromString(TEXT("Cavrnus Spawn Actor From Class"));
        NodeSpawner->DefaultMenuSignature.Category = FText::FromString(TEXT("Cavrnus|Objects"));
        NodeSpawner->DefaultMenuSignature.Tooltip = FText::FromString(TEXT("Spawns an actor with ExposeOnSpawn property support"));

        ActionRegistrar.AddBlueprintAction(NodeClass, NodeSpawner);
    }
}

void UK2Node_CavrnusSpawnActorFromClass::PinDefaultValueChanged(UEdGraphPin* Pin)
{

    if (!Pin)
    {
        return;
    }

    // If ActorClass pin default value changed, update ExposeOnSpawn pins
    if (Pin->PinName == TEXT("ActorClass"))
    {
        UClass* CurrentActorClass = GetActorClassFromPin();
        UE_LOG(LogCavrnusBlueprintModule, Log, TEXT("[PinDefaultValueChanged] ActorClass changed - CurrentActorClass: %s, CachedActorClass: %s"),
            CurrentActorClass ? *CurrentActorClass->GetName() : TEXT("nullptr"),
            CachedActorClass ? *CachedActorClass->GetName() : TEXT("nullptr"));
        
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
    // If DataAsset pin changed, clear ActorClass and update filtering
    else if (Pin->PinName == TEXT("DataAsset"))
    {
        // Clear the ActorClass pin since the valid classes list has changed
        UEdGraphPin* ActorClassPin = FindPin(TEXT("ActorClass"));
        if (ActorClassPin)
        {
            ActorClassPin->DefaultObject = nullptr;
            ActorClassPin->DefaultValue = TEXT("");
            
            // Remove ExposeOnSpawn pins since we no longer have a valid actor class
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
        
        UpdateActorClassPinFiltering();
    }
}

void UK2Node_CavrnusSpawnActorFromClass::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
    // Get input pins
    UEdGraphPin* SpaceConnectionPin = FindPin(TEXT("SpaceConnection"));
    UEdGraphPin* ActorClassPin = FindPin(TEXT("ActorClass"));
    UEdGraphPin* TransformPin = FindPin(TEXT("SpawnTransform"));
    UEdGraphPin* CollisionHandlingPin = FindPin(TEXT("CollisionHandlingOverride"));
    UEdGraphPin* DataAssetPin = FindPin(TEXT("DataAsset"));
    UEdGraphPin* OurExecutePin = FindPin(UEdGraphSchema_K2::PN_Execute);
    UEdGraphPin* OurThenPin = FindPin(UEdGraphSchema_K2::PN_Then);
    UEdGraphPin* OurReturnPin = FindPin(UEdGraphSchema_K2::PN_ReturnValue);

    if (!SpaceConnectionPin || !ActorClassPin || !TransformPin || !CollisionHandlingPin || !DataAssetPin || !OurExecutePin || !OurThenPin || !OurReturnPin)
    {
        CompilerContext.MessageLog.Error(*FString::Printf(TEXT("Missing required pins on @@")), this);
        return;
    }

    // Get ExposeOnSpawn properties to build the array
    UClass* ActorClass = GetActorClassFromPin();
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
    
    // Create function call node for CavrnusSpawnActorFromClassWithArray
    UK2Node_CallFunction* FunctionCallNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
    FunctionCallNode->FunctionReference.SetExternalMember(
        TEXT("CavrnusSpawnActorFromClassWithArray"),
        USpawnedObjectsManager::StaticClass()
    );
    FunctionCallNode->AllocateDefaultPins();

    // Build parameter pin mappings
    TMap<FString, UEdGraphPin*> ParameterPinMappings;
    ParameterPinMappings.Add(TEXT("SpaceConnection"), SpaceConnectionPin);
    ParameterPinMappings.Add(TEXT("ActorClass"), ActorClassPin);
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

FText UK2Node_CavrnusSpawnActorFromClass::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    return FText::FromString(TEXT("Cavrnus Spawn Actor From Class"));
}

FText UK2Node_CavrnusSpawnActorFromClass::GetTooltipText() const
{
    return FText::FromString(TEXT("Spawns an actor of the specified class in the Cavrnus space. ExposeOnSpawn properties are automatically exposed as pins."));
}

UClass* UK2Node_CavrnusSpawnActorFromClass::GetActorClassFromPin() const
{
    UEdGraphPin* ActorClassPin = FindPin(TEXT("ActorClass"));
    if (!ActorClassPin)
    {
        return nullptr;
    }

    // If pin is connected, we can't determine the class at compile time
    if (ActorClassPin->LinkedTo.Num() > 0)
    {
        return nullptr;
    }

    // Try to get from default value
    if (ActorClassPin->DefaultObject)
    {
        if (UClass* Class = Cast<UClass>(ActorClassPin->DefaultObject))
        {
            return Class;
        }
    }

    return nullptr;
}

void UK2Node_CavrnusSpawnActorFromClass::CreateExposeOnSpawnPins()
{
    UClass* ActorClass = GetActorClassFromPin();
    
    // Find where to insert Owner pin - it should appear immediately after CollisionHandling
    // Function signature order: SpaceConnection, DataAsset, ActorClass, Transform, CollisionHandling, Owner, ExposeOnSpawn, Instigator
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
        [this]() { return GetActorClassFromPin(); },
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
            // UE_LOG(LogCavrnusBlueprintModule, Log, TEXT("[CreateExposeOnSpawnPins] ExistingInstigatorPin: ActorClass is valid, bIsActorClass: %d, bShouldRemove: %d"),
            //     bIsActorClass ? 1 : 0, bShouldRemove ? 1 : 0);
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
            UE_LOG(LogCavrnusBlueprintModule, Log, TEXT("[CreateExposeOnSpawnPins] ExistingInstigatorPin: Both are nullptr, bShouldRemove: %d (will remove)"),
                bShouldRemove ? 1 : 0);
        }
        
        if (bShouldRemove)
        {
            UE_LOG(LogCavrnusBlueprintModule, Log, TEXT("[CreateExposeOnSpawnPins] Removing Instigator pin"));
            RemovePin(ExistingInstigatorPin);
        }
        else
        {
            // UE_LOG(LogCavrnusBlueprintModule, Log, TEXT("[CreateExposeOnSpawnPins] Keeping Instigator pin"));
        }
    }
}

void UK2Node_CavrnusSpawnActorFromClass::RemoveExposeOnSpawnPins()
{
    UClass* CurrentClass = GetActorClassFromPin();
    FCavrnusNodeHelpers::RemoveExposeOnSpawnPins(
        this,
        CurrentClass,
        CachedExposeOnSpawnProperties,
        [this]() { return GetActorClassFromPin(); }
    );
}

FEdGraphPinType UK2Node_CavrnusSpawnActorFromClass::GetPinTypeForProperty(FProperty* Prop) const
{
    return FCavrnusNodeHelpers::GetPinTypeForProperty(Prop);
}

TArray<UClass*> UK2Node_CavrnusSpawnActorFromClass::GetValidActorClassesFromDataAsset() const
{
    TArray<UClass*> ValidClasses;

    // Get DataAsset pin
    UEdGraphPin* DataAssetPin = FindPin(TEXT("DataAsset"));
    UCavrnusSpawnableRegistryDataAsset* DataAsset = nullptr;

    // Check if DataAsset pin has a default object set (not connected)
    if (DataAssetPin && DataAssetPin->LinkedTo.Num() == 0)
    {
        // If pin is connected, we can't determine the DataAsset at compile time
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

    // Collect valid actor classes from DataAsset entries
    for (const FCavrnusSpawnableEntry& Entry : DataAsset->GetEntries())
    {
        if (Entry.ActorClass.IsNull())
        {
            continue;
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
            ValidClasses.AddUnique(LoadedClass);
        }
    }

    return ValidClasses;
}

TArray<UClass*> UK2Node_CavrnusSpawnActorFromClass::GetValidActorClasses() const
{
    // If we have cached valid classes, return them
    if (CachedValidClasses.Num() > 0)
    {
        return CachedValidClasses;
    }
    
    // Otherwise, get them fresh from the DataAsset
    return GetValidActorClassesFromDataAsset();
}

void UK2Node_CavrnusSpawnActorFromClass::UpdateActorClassPinFiltering()
{
    UEdGraphPin* ActorClassPin = FindPin(TEXT("ActorClass"));
    if (!ActorClassPin)
    {
        return;
    }

    // Get valid classes from the currently selected DataAsset
    CachedValidClasses = GetValidActorClassesFromDataAsset();

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

    ActorClassPin->PinToolTip = FString::Printf(
        TEXT("Actor class to spawn. Must be one of the classes registered in the selected CavrnusSpawnableRegistryDataAsset.\n\nValid classes: %s"),
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
            // UpdateActorClassPinFiltering] Filtered out invalid path
            continue; // Skip invalid/stale paths
        }
        
        if (!ValidClassesString.IsEmpty()) ValidClassesString += TEXT(",");
        ValidClassesString += ClassPath;
    }

    ActorClassPin->PinToolTip += FString::Printf(TEXT("\n\nValidClasses:%s"), *ValidClassesString);
}