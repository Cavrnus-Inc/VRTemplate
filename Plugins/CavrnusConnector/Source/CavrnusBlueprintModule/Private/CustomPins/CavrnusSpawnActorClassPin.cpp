// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "CustomPins/CavrnusSpawnActorClassPin.h"
#include "CustomPins/CavrnusSpawnableActorClassFilter.h"
#include "BlueprintNodes/K2Node_CavrnusSpawnActorFromClass.h"
#include "BlueprintNodes/K2Node_CavrnusConstructObjectFromClass.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"
#include "ClassViewerModule.h"
#include "ClassViewerFilter.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"  // For FMenuBuilder
#include "CavrnusBlueprintModule.h"  // For LogCavrnusBlueprintModule
#include "Managers/SpawnedObjects/SpawnedObjectsManager.h"
#include "K2Node_CallFunction.h"
#include "AssetManager/DataAssets/CavrnusSpawnableRegistryDataAsset.h"
#include "AssetManager/CavrnusDataAssetManager.h"
#include "Core/Subsystems/CavrnusSubsystem.h"
#include "Core/Contexts/CavrnusEditorContext.h"

void SCavrnusSpawnActorClassPin::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
{
    SetCursor(EMouseCursor::Default);
    bShowLabel = true;
    GraphPinObj = InPin;

    CachedValidClasses = GetValidClassesFromPin(InPin);

    SGraphPin::Construct(SGraphPin::FArguments(), InPin);
}

TArray<UClass*> SCavrnusSpawnActorClassPin::GetValidClassesFromPin(UEdGraphPin* Pin) const
{
    TArray<UClass*> ValidClasses;
    
    if (!Pin)
    {
        return ValidClasses;
    }

    // Look for "ValidClasses:" marker in the tooltip (fallback method)
    FString Tooltip = Pin->PinToolTip;
    int32 MarkerIndex = Tooltip.Find(TEXT("ValidClasses:"));
    if (MarkerIndex == INDEX_NONE)
    {
        return ValidClasses;
    }

    FString ClassesString = Tooltip.Mid(MarkerIndex + 12); // skip "ValidClasses:"
    TArray<FString> ClassPaths;
    ClassesString.ParseIntoArray(ClassPaths, TEXT(","), true);

    for (const FString& Path : ClassPaths)
    {
        FString TrimmedPath = Path.TrimStartAndEnd();
        
        // Remove leading colon if present (can happen from tooltip parsing)
        if (TrimmedPath.StartsWith(TEXT(":")))
        {
            TrimmedPath.RemoveAt(0, 1);
        }
        
        // Filter out garbage paths before attempting to load
        if (TrimmedPath.IsEmpty() || TrimmedPath.StartsWith(TEXT("Class None.")))
        {
           // [GetValidClassesFromPin] Filtered out invalid path before LoadObject
            continue; // Skip invalid/stale paths
        }
        
        // Only attempt to load paths that look valid
        if (UClass* LoadedClass = LoadObject<UClass>(nullptr, *TrimmedPath))
        {
            ValidClasses.AddUnique(LoadedClass);
        }
        else
        {
            UE_LOG(LogCavrnusBlueprintModule, Warning, TEXT("[GetValidClassesFromPin] Failed to load class from path: '%s'"), *TrimmedPath);
        }
    }

    return ValidClasses;
}

void SCavrnusSpawnActorClassPin::OnClassPicked(UClass* NewClass)
{
    // Set to nullptr if "None" was selected, otherwise set the class
    GraphPinObj->DefaultObject = NewClass;
    
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


UClass* SCavrnusSpawnActorClassPin::GetSelectedClass() const
{
    return Cast<UClass>(GraphPinObj->DefaultObject);
}

TSharedRef<SWidget> SCavrnusSpawnActorClassPin::GetDefaultValueWidget()
{
    // Get valid classes directly from the node instead of parsing tooltip
    TArray<UClass*> ValidClasses;

    if (UEdGraphNode* Node = GraphPinObj->GetOwningNode())
    {
        // Try to cast to custom node first
        if (UK2Node_CavrnusSpawnActorFromClass* SpawnNode = Cast<UK2Node_CavrnusSpawnActorFromClass>(Node))
        {
            SpawnNode->UpdateActorClassPinFiltering();
            ValidClasses = SpawnNode->GetValidActorClasses();
        }
        // Check for ConstructObjectFromClass node
        else if (UK2Node_CavrnusConstructObjectFromClass* ConstructNode = Cast<UK2Node_CavrnusConstructObjectFromClass>(Node))
        {
            ConstructNode->UpdateObjectClassPinFiltering();
            ValidClasses = ConstructNode->GetValidObjectClasses();
        }
        // If that fails, check if it's a UK2Node_CallFunction calling our function
        else if (UK2Node_CallFunction* CallFunctionNode = Cast<UK2Node_CallFunction>(Node))
        {
            // Check if this node is calling CavrnusSpawnActorFromClass
            FName FunctionName = CallFunctionNode->FunctionReference.GetMemberName();
            UClass* FunctionClass = CallFunctionNode->FunctionReference.GetMemberParentClass();
            
            if (FunctionName == TEXT("CavrnusSpawnActorFromClass") && 
                FunctionClass == USpawnedObjectsManager::StaticClass())
            {
                // Get DataAsset pin
                UEdGraphPin* DataAssetPin = CallFunctionNode->FindPin(TEXT("DataAsset"));
                UCavrnusSpawnableRegistryDataAsset* DataAsset = nullptr;
                
                if (DataAssetPin && DataAssetPin->LinkedTo.Num() == 0 && DataAssetPin->DefaultObject)
                {
                    DataAsset = Cast<UCavrnusSpawnableRegistryDataAsset>(DataAssetPin->DefaultObject);
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
                
                if (DataAsset)
                {
                    // Collect valid actor classes from DataAsset
                    for (const FCavrnusSpawnableEntry& Entry : DataAsset->GetEntries())
                    {
                        if (Entry.ActorClass.IsNull())
                        {
                            continue;
                        }
                        
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
                        
                        if (LoadedClass && LoadedClass->IsChildOf(AActor::StaticClass()))
                        {
                            ValidClasses.AddUnique(LoadedClass);
                        }
                    }
                }
            }
        }
    }

    // Fallback: try to get from tooltip if node method didn't work
    if (ValidClasses.Num() == 0)
    {
        ValidClasses = GetValidClassesFromPin(GraphPinObj);
    }

    // Create a combo button with a dropdown menu of filtered classes
    return SNew(SComboButton)
        .ContentPadding(FMargin(2.0f, 2.0f))
        .ButtonContent()
        [
            SNew(STextBlock)
                .Text_Lambda([this]()
                    {
                        UClass* SelectedClass = GetSelectedClass();
                        if (SelectedClass)
                        {
                            return FText::FromString(SelectedClass->GetName());
                        }
                        return FText::FromString(TEXT("None"));
                    })
        ]
        .OnGetMenuContent_Lambda([this, ValidClasses]()
            {
                // Refresh valid classes from node (in case they changed)
                TArray<UClass*> CurrentValidClasses = ValidClasses;

                if (UEdGraphNode* Node = GraphPinObj->GetOwningNode())
                {
                    if (UK2Node_CavrnusSpawnActorFromClass* SpawnNode = Cast<UK2Node_CavrnusSpawnActorFromClass>(Node))
                    {
                        SpawnNode->UpdateActorClassPinFiltering();
                        CurrentValidClasses = SpawnNode->GetValidActorClasses();
                    }
                }

                // Create a menu builder
                FMenuBuilder MenuBuilder(true, nullptr);

                // Add "None" option
                MenuBuilder.AddMenuEntry(
                    FText::FromString(TEXT("None")),
                    FText::GetEmpty(),
                    FSlateIcon(),
                    FUIAction(
                        FExecuteAction::CreateLambda([this]() { OnClassPicked(nullptr); })
                    )
                );

                // Add a separator if we have classes
                if (CurrentValidClasses.Num() > 0)
                {
                    MenuBuilder.AddMenuSeparator();
                }

                // Add each valid class as a menu entry
                for (UClass* ValidClass : CurrentValidClasses)
                {
                    if (!ValidClass)
                    {
                        continue;
                    }

                    FString ClassName = ValidClass->GetName();
                    FText ClassDisplayName = FText::FromString(ClassName);

                    // Capture ValidClass in the lambda
                    UClass* CapturedClass = ValidClass;
                    MenuBuilder.AddMenuEntry(
                        ClassDisplayName,
                        FText::GetEmpty(),
                        FSlateIcon(),
                        FUIAction(
                            FExecuteAction::CreateLambda([this, CapturedClass]() { OnClassPicked(CapturedClass); })
                        )
                    );
                }

                // If no valid classes, show a message
                if (CurrentValidClasses.Num() == 0)
                {
                    MenuBuilder.AddMenuEntry(
                        FText::FromString(TEXT("No valid classes found in DataAsset")),
                        FText::GetEmpty(),
                        FSlateIcon(),
                        FUIAction() // No action - disabled
                    );
                }

                return MenuBuilder.MakeWidget();
            });
}
