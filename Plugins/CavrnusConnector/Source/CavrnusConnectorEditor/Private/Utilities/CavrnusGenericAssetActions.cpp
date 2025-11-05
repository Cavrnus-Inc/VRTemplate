// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Utilities/CavrnusGenericAssetActions.h"
#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"
#include "Logging/LogMacros.h"
#include "CavrnusSubsystem.h"
#include "AssetManager/CavrnusDataAssetManager.h"
#include "Misc/Optional.h"
#include "AssetRegistry/AssetData.h"
#include "Widgets/SWindow.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Framework/Application/SlateApplication.h"
#include "AssetManager/DataAssets/CavrnusSpawnableActorsDataAsset.h"
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 1
#include "AssetRegistryModule.h"
#else
#include "AssetRegistry/AssetRegistryModule.h"
#endif
#define LOCTEXT_NAMESPACE "CavrnusGenericAssetActions"

#define UE_USES_SOFT_OBJECT_PATH_FOR_ASSET_LOOKUP \
    (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 5)

void ShowStringInputDialog(const FString& Prompt, TFunction<void(const FString&)> OnTextEntered)
{
    TSharedPtr<SEditableTextBox> TextBox;
    TSharedRef<SWindow> Dialog = SNew(SWindow)
        .Title(FText::FromString(Prompt))
        .ClientSize(FVector2D(400, 100))
        .SupportsMinimize(false)
        .SupportsMaximize(false);

    TWeakPtr<SWindow> WeakDialog = Dialog;

    Dialog->SetContent(
        SNew(SVerticalBox)
        + SVerticalBox::Slot().Padding(10)
        [
            SAssignNew(TextBox, SEditableTextBox)
                .HintText(FText::FromString("Enter text here..."))
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(10)
        [
            SNew(SButton)
                .Text(FText::FromString("OK"))
                .OnClicked_Lambda([WeakDialog, TextBox, OnTextEntered]()
                    {
                        if (WeakDialog.IsValid())
                        {
                            OnTextEntered(TextBox->GetText().ToString());
                            FSlateApplication::Get().RequestDestroyWindow(WeakDialog.Pin().ToSharedRef());
                        }
                        return FReply::Handled();
                    })
        ]
    );

    FSlateApplication::Get().AddModalWindow(Dialog, nullptr);
}


template<typename KeyType, typename ValueType>
TOptional<KeyType> FindKeyForValue(const TMap<KeyType, ValueType>& Map, const ValueType& TargetValue)
{
    for (const TPair<KeyType, ValueType>& Pair : Map)
    {
        if (Pair.Value == TargetValue)
        {
            return Pair.Key;
        }
    }
    return TOptional<KeyType>(); // Not found
}

bool FCavrnusGenericAssetActions::HasActions(const TArray<UObject*>& InObjects) const
{
    return true;
}

bool IsSpawnableObject(UObject* Object)
{
    if (!Object)
    {
        return false;
    }

    UClass* ObjectClass = Object->GetClass();

    // Check if it's a Blueprint-generated actor class
    if (UBlueprint* BP = Cast<UBlueprint>(Object))
    {
        UClass* GeneratedClass = BP->GeneratedClass;
        return GeneratedClass &&
            GeneratedClass->IsChildOf(AActor::StaticClass()) &&
            !GeneratedClass->HasAnyClassFlags(CLASS_Abstract);
    }

    // Check if it's a native actor class
    if (ObjectClass->IsChildOf(AActor::StaticClass()) &&
        !ObjectClass->HasAnyClassFlags(CLASS_Abstract))
    {
        return true;
    }

    // Check if it's a known asset type that can be placed via actor factories
    if (Object->IsA(UStaticMesh::StaticClass()) ||
        Object->IsA(USkeletalMesh::StaticClass()) ||
        Object->IsA(UMaterial::StaticClass()))
    {
        return true; // These are spawnable via known factories like StaticMeshActor
    }

    return false;
}

// Build soft references from an arbitrary Content Browser object.
// Returns true if it could be represented in our spawnable schema.
static bool BuildSpawnableRefsFromObject(
    UObject* Obj,
    TSoftClassPtr<AActor>& OutActorClass,
    TSoftObjectPtr<UStaticMesh>& OutStaticMesh)
{
    OutActorClass = nullptr;
    OutStaticMesh = nullptr;

    if (!Obj) return false;

    // Blueprint asset → generated actor class
    if (UBlueprint* BP = Cast<UBlueprint>(Obj))
    {
        if (BP->GeneratedClass &&
            BP->GeneratedClass->IsChildOf(AActor::StaticClass()) &&
            !BP->GeneratedClass->HasAnyClassFlags(CLASS_Abstract))
        {
            OutActorClass = TSoftClassPtr<AActor>(BP->GeneratedClass);
            return true;
        }
        return false;
    }

    // Native actor class
    if (UClass* AsClass = Cast<UClass>(Obj))
    {
        if (AsClass->IsChildOf(AActor::StaticClass()) &&
            !AsClass->HasAnyClassFlags(CLASS_Abstract))
        {
            OutActorClass = TSoftClassPtr<AActor>(AsClass);
            return true;
        }
        return false;
    }

    // Static mesh asset
    if (UStaticMesh* Mesh = Cast<UStaticMesh>(Obj))
    {
        OutStaticMesh = TSoftObjectPtr<UStaticMesh>(Mesh);
        return true;
    }

    return false;
}

// Find an entry by matching the underlying soft reference path to the object
static int32 FindEntryIndexForObject(const UCavrnusSpawnableActorsDataAsset* DataAsset, UObject* Obj)
{
    if (!DataAsset || !Obj) return INDEX_NONE;

    if (UBlueprint* BP = Cast<UBlueprint>(Obj))
    {
        Obj = BP->GeneratedClass;
    }

    const FString ObjPath = Obj->GetPathName();

    const TArray<FCavrnusSpawnableEntry>& Entries = DataAsset->GetEntries();
    for (int32 i = 0; i < Entries.Num(); ++i)
    {
        const FCavrnusSpawnableEntry& Entry = Entries[i];

        FString ActorEntryPath = Entry.ActorClass.ToSoftObjectPath().ToString();
        FString StaticEntryPath = Entry.StaticMesh.ToSoftObjectPath().ToString();
        if (ActorEntryPath == ObjPath)
        {
            return i;
        }

        if (StaticEntryPath == ObjPath)
        {
            return i;
        }
    }
    return INDEX_NONE;
}

void FCavrnusGenericAssetActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
    UCavrnusSpawnableActorsDataAsset* DataAsset = UCavrnusSubsystem::Get()->Services->Get<UCavrnusDataAssetManager>()->GetAsset<UCavrnusSpawnableActorsDataAsset>();
    if (!DataAsset) return;

    // Decide whether to show Add/Remove per selected object
    for (UObject* Obj : InObjects)
    {
        if (!IsSpawnableObject(Obj)) continue;

        const int32 ExistingIdx = FindEntryIndexForObject(DataAsset, Obj);
        if (ExistingIdx != INDEX_NONE)
        {
            MenuBuilder.AddMenuEntry(
                LOCTEXT("RemoveFromSpawnList", "Remove From Cavrnus Spawn List"),
                LOCTEXT("RemoveFromSpawnListTooltip", "Removes this asset from the Cavrnus spawnable list"),
                FSlateIcon(),
                FUIAction(FExecuteAction::CreateSP(this, &FCavrnusGenericAssetActions::ExecuteRemoveFromSpawnList, InObjects))
            );
        }
        else
        {
            MenuBuilder.AddMenuEntry(
                LOCTEXT("AddToSpawnList", "Add to Cavrnus Spawn List"),
                LOCTEXT("AddToSpawnListTooltip", "Adds this asset to the Cavrnus spawnable list"),
                FSlateIcon(),
                FUIAction(FExecuteAction::CreateSP(this, &FCavrnusGenericAssetActions::ExecuteAddToSpawnList, InObjects))
            );
        }
    }

    // Always offer "Print list"
    MenuBuilder.AddMenuEntry(
        LOCTEXT("PrintSpawnList", "Print Cavrnus Spawn List"),
        LOCTEXT("PrintSpawnListTooltip", "Prints assets in the Cavrnus spawnable list"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateSP(this, &FCavrnusGenericAssetActions::ExecutePrintSpawnList, InObjects))
    );
}

void PrintClassHierarchy(UObject* Obj)
{
    if (!Obj) return;
    UClass* Class = Obj->GetClass();
    FString Hierarchy;
    while (Class)
    {
        if (!Hierarchy.IsEmpty())
        {
            Hierarchy = " -> " + Hierarchy;
        }
        Hierarchy = Class->GetName() + Hierarchy;
        Class = Class->GetSuperClass();
    }
    UE_LOG(LogTemp, Log, TEXT("Class Hierarchy for %s: %s"), *Obj->GetName(), *Hierarchy);
}

TOptional<FAssetData> FCavrnusGenericAssetActions::GetAssetData(UObject* Obj)
{
    if (!Obj)
        return TOptional<FAssetData>();
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
    if (!&AssetRegistryModule)
        return TOptional<FAssetData>();
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1) // Soft Object Path Change in GetAssetByObjectPath()
    FSoftObjectPath ObjectPath(Obj);
    FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(ObjectPath);
#else
    FName ObjectPathName(*Obj->GetPathName());
    FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(ObjectPathName);
#endif
    return AssetData.IsValid() ? TOptional<FAssetData>(AssetData) : TOptional<FAssetData>();
}

void FCavrnusGenericAssetActions::ExecuteAddToSpawnList(TArray<UObject*> Objects)
{
    for (UObject* Obj : Objects)
    {
        if (!IsSpawnableObject(Obj)) continue;
        PrintClassHierarchy(Obj);

        ShowStringInputDialog(TEXT("Enter Cavrnus Spawn Identifier :"),
            [Obj](const FString& UserInput)
            {
                if (UserInput.IsEmpty()) return;

                UCavrnusSpawnableActorsDataAsset* DataAsset =
                    UCavrnusSubsystem::Get()->Services->Get<UCavrnusDataAssetManager>()->GetAsset<UCavrnusSpawnableActorsDataAsset>();
                if (!DataAsset) return;

                TSoftClassPtr<AActor> ActorClass;
                TSoftObjectPtr<UStaticMesh> StaticMesh;
                if (!BuildSpawnableRefsFromObject(Obj, ActorClass, StaticMesh))
                {
                    UE_LOG(LogTemp, Warning, TEXT("Object is not a supported Cavrnus Spawnable: %s"), *Obj->GetName());
                    return;
                }

                const FName Identifier(*UserInput);
                if (ActorClass.IsValid())
                {
                    DataAsset->AddSpawnableActor(Identifier, ActorClass);
                }
                else if (StaticMesh.IsValid())
                {
                    DataAsset->AddSpawnableMesh(Identifier, StaticMesh);
                }
            });
    }
}

void FCavrnusGenericAssetActions::ExecuteRemoveFromSpawnList(TArray<UObject*> Objects)
{
    UCavrnusSpawnableActorsDataAsset* DataAsset =
        UCavrnusSubsystem::Get()->Services->Get<UCavrnusDataAssetManager>()->GetAsset<UCavrnusSpawnableActorsDataAsset>();
    if (!DataAsset) return;

    for (UObject* Obj : Objects)
    {
        const int32 ExistingIdx = FindEntryIndexForObject(DataAsset, Obj);
        if (ExistingIdx == INDEX_NONE)
        {
            UE_LOG(LogTemp, Log, TEXT("Asset not found in Cavrnus spawn list: %s"), *Obj->GetName());
            continue;
        }

        // Remove by key (keeps API simple)
        const FName Key = DataAsset->GetEntries()[ExistingIdx].Key;
        DataAsset->RemoveSpawnable(Key);
    }
}

void FCavrnusGenericAssetActions::ExecutePrintSpawnList(TArray<UObject*> /*Objects*/)
{
    UCavrnusSpawnableActorsDataAsset* DataAsset =
        UCavrnusSubsystem::Get()->Services->Get<UCavrnusDataAssetManager>()->GetAsset<UCavrnusSpawnableActorsDataAsset>();
    if (!DataAsset) return;

    for (const FCavrnusSpawnableEntry& Entry : DataAsset->GetEntries())
    {
        FString ValueStr = TEXT("<none>");

        if (Entry.ActorClass.IsValid())
        {
            if (UClass* C = Entry.ActorClass.Get())
            {
                ValueStr = C->GetPathName();
            }
            else
            {
                ValueStr = Entry.ActorClass.ToSoftObjectPath().ToString();
            }
        }
        else if (Entry.StaticMesh.IsValid())
        {
            if (UStaticMesh* M = Entry.StaticMesh.Get())
            {
                ValueStr = M->GetPathName();
            }
            else
            {
                ValueStr = Entry.StaticMesh.ToSoftObjectPath().ToString();
            }
        }

        UE_LOG(LogTemp, Log, TEXT("%s : %s"), *Entry.Key.ToString(), *ValueStr);
    }
}
#undef LOCTEXT_NAMESPACE
