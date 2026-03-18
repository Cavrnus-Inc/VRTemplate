#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataAsset.h"
#include "AssetManager/CavrnusBaseDataAsset.h"
#if WITH_EDITOR
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#endif
#include "Managers/CavrnusService.h"
#include "CavrnusDataAssetManager.generated.h"

class UCavrnusMigrationManager;

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusDataAssetManager : public UCavrnusService
{
    GENERATED_BODY()

public:
    virtual void Initialize() override;

    template<typename T>
    T* LoadAsset()
    {
        static_assert(TIsDerivedFrom<T, UCavrnusBaseDataAsset>::IsDerived, "T must derive from UCavrnusBaseDataAsset");

        UClass* TypeClass = T::StaticClass();
        check(TypeClass);

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
        return Loaded;
    }

    template<typename T>
    T* GetAsset() const
    {
        if (!LoadedAssets.Num())
        {
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

private:
    bool CheckUnrealVersionNewerThanEngine(const FString& PackageFilename);
    bool CheckUnrealVersionCompatible(const FString& PackageFilename);

    UPROPERTY()
    TMap<UClass*, FString> AssetPaths;
    UPROPERTY()
    TMap<UClass*, UCavrnusBaseDataAsset*> LoadedAssets;

    FString SourceDataAssetPath = TEXT("/CavrnusConnector/SourceDataAssets");
    FString DestinationDataAssetPath = TEXT("/Game/Cavrnus/DataAssets");

    UCavrnusBaseDataAsset* CopyAssetIfMissing(UObject* SourceAsset, const FString& DestinationPath);
    bool SavePackageVersionSafe(UPackage* Package, UObject* Asset, const FString& PackageFilename);

    UPROPERTY()
    UCavrnusMigrationManager* MigrationManager = nullptr;

#if WITH_EDITOR
    void CheckAndNotifyVersionUpdates();
    void UpdateDataAsset(UCavrnusBaseDataAsset* SourceAsset, UCavrnusBaseDataAsset* ProjectCopy);
    void BackupDataAsset(UCavrnusBaseDataAsset* Asset, int32 Version);

    FString BackupDataAssetPath = TEXT("/Game/Cavrnus/DataAssetBackups");
#endif
};
