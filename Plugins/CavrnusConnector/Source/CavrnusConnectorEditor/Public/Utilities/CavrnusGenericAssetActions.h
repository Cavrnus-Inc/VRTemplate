// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

#include "Misc/Optional.h"
#include "AssetRegistry/AssetData.h"
#if WITH_EDITOR
#include "Subsystems/AssetEditorSubsystem.h"
#include "Editor.h" // for GEditor
#endif
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 1
#include "AssetRegistryModule.h"
#else
#include "AssetRegistry/AssetRegistryModule.h"
#endif

class FCavrnusGenericAssetActions : public FAssetTypeActions_Base
{
public:
    FCavrnusGenericAssetActions(UClass* InClass, FText InName, uint32 InCategories, const TSharedPtr<IAssetTypeActions>& InFallback)
        : SupportedClass(InClass)
        , NativeName(InName)
        , NativeCategories(InCategories)
        , FallbackActions(InFallback)
    {
        UE_LOG(LogTemp, Log, TEXT("XXX - FCavrnusGenericAssetActions for% s — Fallback is% s"),
            *InClass->GetName(),
            InFallback.IsValid() ? *InFallback->GetName().ToString() : TEXT("None"));

    }
private:
    // Action logic (implemented in .cpp)
    void ExecuteAddToSpawnList(TArray<UObject*> Objects);
    void ExecuteRemoveFromSpawnList(TArray<UObject*> Objects);
    void ExecutePrintSpawnList(TArray<UObject*> Objects);

    UClass* SupportedClass;
    FText NativeName;
    uint32 NativeCategories;
    TSharedPtr<IAssetTypeActions> FallbackActions;

public:
    // Registers Cavrnus overrides for selected asset classes

    virtual FText GetName() const override { return NativeName; }
    virtual uint32 GetCategories() override { return NativeCategories; }
    virtual UClass* GetSupportedClass() const override { return SupportedClass; }

    // Whether this asset type has context menu actions
    virtual bool HasActions(const TArray<UObject*>& InObjects) const override;

    // Adds custom menu entries
    virtual void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override;

    virtual FColor GetTypeColor() const override { return FColor::Emerald; }

    virtual void OpenAssetEditor(
        const TArray<UObject*>& InObjects,
        TSharedPtr<class IToolkitHost> EditWithinLevelEditor) override
    {
#if WITH_EDITOR
        if (FallbackActions.IsValid() && FallbackActions.Get() != this)
        {
            FallbackActions->OpenAssetEditor(InObjects, EditWithinLevelEditor);
        }
        else if (GEditor)
        {
            if (UAssetEditorSubsystem* AES = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
            {
                for (UObject* Obj : InObjects)
                {
                    if (IsValid(Obj))
                        AES->OpenEditorForAsset(Obj);
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Unable to open object - Invalid"));
                    }
                }
            }
        }
#endif
    }

    TOptional<FAssetData> GetAssetData(UObject* Obj);

};
