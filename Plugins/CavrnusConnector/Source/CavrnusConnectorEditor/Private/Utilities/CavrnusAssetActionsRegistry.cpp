#include "Utilities/CavrnusAssetActionsRegistry.h"
#include "Utilities/CavrnusGenericAssetActions.h"

#include "AssetToolsModule.h"
#include "Editor.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "TimerManager.h"

TArray<TSharedPtr<IAssetTypeActions>> FCavrnusAssetActionsRegistry::Registered;

void FCavrnusAssetActionsRegistry::RegisterOverrides()
{
    TArray<UClass*> TargetClasses = {
        UStaticMesh::StaticClass(),
        UBlueprint::StaticClass(),
        UMaterial::StaticClass(),
        // Add more asset types as needed
    };

    for (UClass* ClassType : TargetClasses)
    {
        TryRegister(ClassType);
    }
}

void FCavrnusAssetActionsRegistry::TryRegister(UClass* ClassType)
{
    IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
    const TWeakPtr<IAssetTypeActions> Weak = AssetTools.GetAssetTypeActionsForClass(ClassType);
    TSharedPtr<IAssetTypeActions> Original = Weak.Pin();

    if (!Original.IsValid())
    {
        FTimerHandle RetryHandle;
        GEditor->GetTimerManager()->SetTimer(RetryHandle, [ClassType]()
        {
            TryRegister(ClassType);
        }, 0.1f, false);
        return;
    }

    TSharedPtr<IAssetTypeActions> Override = MakeShared<FCavrnusGenericAssetActions>(
        ClassType,
        Original->GetName(),
        Original->GetCategories(),
        Original
    );

    AssetTools.RegisterAssetTypeActions(Override.ToSharedRef());
    Registered.Add(Override);
}

void FCavrnusAssetActionsRegistry::UnregisterOverrides()
{
    if (!FModuleManager::Get().IsModuleLoaded("AssetTools"))
        return;

    IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();

    for (const TSharedPtr<IAssetTypeActions>& Override : Registered)
    {
        AssetTools.UnregisterAssetTypeActions(Override.ToSharedRef());
    }

    Registered.Empty();
}

