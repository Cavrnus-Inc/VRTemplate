// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "CustomPins/CavrnusSpawnActorIdPin.h"
#include "BlueprintNodes/K2Node_CavrnusSpawnActorById.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "CavrnusBlueprintModule.h"
#include "Managers/SpawnedObjects/SpawnedObjectsManager.h"
#include "K2Node_CallFunction.h"
#include "AssetManager/DataAssets/CavrnusSpawnableRegistryDataAsset.h"
#include "AssetManager/CavrnusDataAssetManager.h"
#include "Core/Subsystems/CavrnusSubsystem.h"
#include "Core/Contexts/CavrnusEditorContext.h"
#include "Kismet2/BlueprintEditorUtils.h"

void SCavrnusSpawnActorIdPin::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
{
    SetCursor(EMouseCursor::Default);
    bShowLabel = true;
    GraphPinObj = InPin;

    CachedValidIds = GetValidIdsFromNode();

    SGraphPin::Construct(SGraphPin::FArguments(), InPin);
}

TArray<FName> SCavrnusSpawnActorIdPin::GetValidIdsFromNode() const
{
    TArray<FName> ValidIds;

    if (!GraphPinObj)
    {
        return ValidIds;
    }

    if (UEdGraphNode* Node = GraphPinObj->GetOwningNode())
    {
        // Try to cast to custom node first
        if (UK2Node_CavrnusSpawnActorById* SpawnNode = Cast<UK2Node_CavrnusSpawnActorById>(Node))
        {
            ValidIds = SpawnNode->GetValidIds();
        }
        // If that fails, check if it's a UK2Node_CallFunction calling our function
        else if (UK2Node_CallFunction* CallFunctionNode = Cast<UK2Node_CallFunction>(Node))
        {
            // Check if this node is calling CavrnusSpawnActorById
            FName FunctionName = CallFunctionNode->FunctionReference.GetMemberName();
            UClass* FunctionClass = CallFunctionNode->FunctionReference.GetMemberParentClass();
            
            if (FunctionName == TEXT("CavrnusSpawnActorById") && 
                FunctionClass == USpawnedObjectsManager::StaticClass())
            {
                // Get DataAsset pin
                UEdGraphPin* DataAssetPin = CallFunctionNode->FindPin(TEXT("DataAsset"));
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
                    // Collect valid IDs from DataAsset entries
                    for (const FCavrnusSpawnableEntry& Entry : DataAsset->GetEntries())
                    {
                        if (!Entry.Key.IsNone())
                        {
                            ValidIds.AddUnique(Entry.Key);
                        }
                    }
                }
            }
        }
    }

    return ValidIds;
}

void SCavrnusSpawnActorIdPin::OnIdPicked(FName NewId)
{
    // Set the default value to the selected ID (or empty string for "None")
    if (NewId.IsNone())
    {
        GraphPinObj->DefaultValue = TEXT("");
    }
    else
    {
        GraphPinObj->DefaultValue = NewId.ToString();
    }
    
    // Notify the node that the pin value changed
    if (UEdGraphNode* Node = GraphPinObj->GetOwningNode())
    {
        Node->PinDefaultValueChanged(GraphPinObj);
        
        // Notify the graph that the node structure has changed
        if (UEdGraph* Graph = Node->GetGraph())
        {
            Graph->NotifyGraphChanged();
            if (UBlueprint* BP = FBlueprintEditorUtils::FindBlueprintForGraph(Graph))
            {
                FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP);
            }
        }
    }
}

FName SCavrnusSpawnActorIdPin::GetSelectedId() const
{
    if (!GraphPinObj || GraphPinObj->DefaultValue.IsEmpty())
    {
        return NAME_None;
    }
    return FName(*GraphPinObj->DefaultValue);
}

TSharedRef<SWidget> SCavrnusSpawnActorIdPin::GetDefaultValueWidget()
{
    // Get valid IDs directly from the node
    TArray<FName> ValidIds = GetValidIdsFromNode();

    // Create a combo button with a dropdown menu of filtered IDs
    return SNew(SComboButton)
        .ContentPadding(FMargin(2.0f, 2.0f))
        .ButtonContent()
        [
            SNew(STextBlock)
                .Text_Lambda([this]()
                {
                    FName SelectedId = GetSelectedId();
                    if (!SelectedId.IsNone())
                    {
                        return FText::FromName(SelectedId);
                    }
                    return FText::FromString(TEXT("None"));
                })
        ]
        .OnGetMenuContent_Lambda([this, ValidIds]()
        {
            // Refresh valid IDs from node (in case they changed)
            TArray<FName> CurrentValidIds = GetValidIdsFromNode();
            
            // Create a menu builder
            FMenuBuilder MenuBuilder(true, nullptr);
            
            // Add "None" option
            MenuBuilder.AddMenuEntry(
                FText::FromString(TEXT("None")),
                FText::GetEmpty(),
                FSlateIcon(),
                FUIAction(
                    FExecuteAction::CreateLambda([this]() { OnIdPicked(NAME_None); })
                )
            );
            
            // Add a separator if we have IDs
            if (CurrentValidIds.Num() > 0)
            {
                MenuBuilder.AddMenuSeparator();
            }
            
            // Add each valid ID as a menu entry
            for (const FName& ValidId : CurrentValidIds)
            {
                if (ValidId.IsNone())
                {
                    continue;
                }
                
                FText IdDisplayName = FText::FromName(ValidId);
                
                // Capture ValidId in the lambda
                FName CapturedId = ValidId;
                MenuBuilder.AddMenuEntry(
                    IdDisplayName,
                    FText::GetEmpty(),
                    FSlateIcon(),
                    FUIAction(
                        FExecuteAction::CreateLambda([this, CapturedId]() { OnIdPicked(CapturedId); })
                    )
                );
            }
            
            // If no valid IDs, show a message
            if (CurrentValidIds.Num() == 0)
            {
                MenuBuilder.AddMenuEntry(
                    FText::FromString(TEXT("No valid IDs found in DataAsset")),
                    FText::GetEmpty(),
                    FSlateIcon(),
                    FUIAction() // No action - disabled
                );
            }
            
            return MenuBuilder.MakeWidget();
        });
}

