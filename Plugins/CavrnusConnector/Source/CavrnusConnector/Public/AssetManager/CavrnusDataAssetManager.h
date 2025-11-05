#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataAsset.h"
#include "AssetManager/CavrnusBaseDataAsset.h"
#include "AssetManager/CavrnusDataAssetRegistry.h"
#if WITH_EDITOR
#include "AssetToolsModule.h"   // FAssetToolsModule
#include "IAssetTools.h"        // IAssetTools interface
#include "AssetRegistry/AssetRegistryModule.h"
#endif
#include "Managers/CavrnusService.h"
#include "CavrnusDataAssetManager.generated.h"

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusDataAssetManager : public UCavrnusService
{
    GENERATED_BODY()

public:
    static UCavrnusDataAssetManager* Get(UObject* ReferenceObject)
    {
        UCavrnusDataAssetManager* Singleton = GetMutableDefault<UCavrnusDataAssetManager>();
        static bool bInitialized = false;
        if (!bInitialized)
        {
            Singleton->Initialize();
            bInitialized = true;
        }
        return Singleton;
    }

    virtual void Initialize() override;
    void BuildOrUpdateRegistry();

    void ScanAndPurgeTransientAssets();

    template<typename T>
    T* LoadAsset()
    {
        static_assert(TIsDerivedFrom<T, UCavrnusBaseDataAsset>::IsDerived, "T must derive from UCavrnusBaseDataAsset");

        UClass* TypeClass = T::StaticClass();
        check(TypeClass); // Ensure StaticClass() is valid

        if (UCavrnusBaseDataAsset* Found = LoadedAssets.FindRef(TypeClass))
        {
            return Cast<T>(Found);
        }

        const FString* Path = AssetPaths.Find(TypeClass);
        if (!Path || Path->IsEmpty())
        {
            UE_LOG(LogTemp, Error, TEXT("No asset path registered for type %s"), *TypeClass->GetName());
            return nullptr;
        }

        UObject* RawObject = LoadObject<T>(nullptr, **Path);
        if (!RawObject)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to load asset of type %s from path: %s"), *TypeClass->GetName(), **Path);
            return nullptr;
        }

        T* Loaded = Cast<T>(RawObject);
        if (!Loaded)
        {
            UE_LOG(LogTemp, Error, TEXT("Loaded object from path %s is not of expected type %s (actual: %s)"),
                **Path, *TypeClass->GetName(), *RawObject->GetClass()->GetName());
            return nullptr;
        }

        LoadedAssets.Add(TypeClass, Loaded);
        UE_LOG(LogTemp, Log, TEXT("Successfully loaded and cached asset %s: %p"), *TypeClass->GetName(), Loaded);
        return Loaded;
    }


    template<typename T>
    T* GetAsset() const
    {
        if (!LoadedAssets.Num())
        {
            UE_LOG(LogTemp, Error, TEXT("LoadedAssets is empty or uninitialized"));
            return nullptr;
        }
        static_assert(TIsDerivedFrom<T, UCavrnusBaseDataAsset>::IsDerived, "T must derive from UCavrnusBaseDataAsset");
        UClass* TypeClass = T::StaticClass();

        if (UCavrnusBaseDataAsset* Found = LoadedAssets.FindRef(TypeClass))
        {
            return Cast<T>(Found);
        }

        return nullptr;
    };

    TArray<TSoftObjectPtr<UCavrnusBaseDataAsset>> GetSourceAssets(const FString& AssetPath) const;

    void LoadRuntimeAssets();

    void InitializeFromRegistry(UCavrnusDataAssetRegistry* Registry);

    UCavrnusBaseDataAsset* EnsureAssetByClass(UClass* AssetClass, const FString& AssetPath);

private:
    bool CheckUnrealVersionNewerThanEngine(const FString& PackageFilename);

    UPROPERTY()
    TMap<UClass*, FString> AssetPaths;
    UPROPERTY()
    TMap<UClass*, UCavrnusBaseDataAsset*> LoadedAssets;

    FString SourceDataAssetPath = TEXT("/CavrnusConnector/SourceDataAssets");
    FString DestinationDataAssetPath = TEXT("/Game/Cavrnus/DataAssets");
    static constexpr const TCHAR* RuntimeRegistryName = TEXT("RuntimeRegistry");
    UPROPERTY()
    TArray<TSoftObjectPtr<UCavrnusBaseDataAsset>> DiscoveredAssets;
    UCavrnusBaseDataAsset* CopyAssetIfMissing(UObject* SourceAsset, const FString& DestinationPath);

    bool SavePackageVersionSafe(UPackage* Package, UObject* Asset, const FString& PackageFilename);
};

