#include "AssetManager/CavrnusDataAssetManager.h"
#include "AssetManager/DataAssets/CavrnusSpawnableActorsDataAsset.h"
#include "CavrnusConnectorModule.h"
#include "HAL/FileManager.h"           // For IFileManager
#include "Misc/Paths.h"                // For FPaths
#include "Serialization/Archive.h"    // For FArchive
#include "UObject/Package.h"          // For GPackageFileUEVersion
#include "UObject/UObjectGlobals.h"

#if WITH_EDITOR && ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
// Modern API: FSavePackageArgs (UE5.1+)
#include "PackageHelperFunctions.h"
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
#include "UObject/SavePackage.h"
#else
// Legacy API: SavePackage (UE5.0)
#include "PackageTools.h"
#endif
#endif

void UCavrnusDataAssetManager::Initialize()
{
    Super::Initialize();
#if WITH_EDITOR
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    AssetRegistryModule.Get().ScanPathsSynchronous({ *SourceDataAssetPath }, true);

    TArray<FAssetData> AssetDataList;
    AssetRegistryModule.Get().GetAssetsByPath(*SourceDataAssetPath, AssetDataList, true);

    for (const FAssetData& AssetData : AssetDataList)
    {
        const FString AssetName = AssetData.AssetName.ToString();
        const FString DestinationPath = DestinationDataAssetPath + TEXT("/") + AssetName;
        UClass* AssetClass = AssetData.GetClass();

        if (!AssetClass || !AssetClass->IsChildOf(UCavrnusBaseDataAsset::StaticClass()))
        {
            UE_LOG(LogCavrnusConnector, Warning, TEXT("Skipping asset %s: invalid or non-CavrnusBaseDataAsset class"), *AssetName);
            continue;
        }

        // Check if destination already exists
        const FString DestObjectPath = DestinationPath + TEXT(".") + AssetName;
        UObject* Existing = StaticLoadObject(AssetClass, nullptr, *DestObjectPath);
        if (UCavrnusBaseDataAsset* ExistingTyped = Cast<UCavrnusBaseDataAsset>(Existing))
        {
            LoadedAssets.Add(AssetClass, ExistingTyped);
            continue; // Already exists, skip ensure + copy
        }

        // Load source asset
        TSoftObjectPtr<UCavrnusBaseDataAsset> SourceRef(AssetData.ToSoftObjectPath());
        UCavrnusBaseDataAsset* SourceAsset = SourceRef.LoadSynchronous();
        if (!SourceAsset)
        {
            UE_LOG(LogCavrnusConnector, Warning, TEXT("Failed to load source asset %s"), *AssetData.ToSoftObjectPath().ToString());
            continue;
        }

        // Duplicate source asset to destination
        UCavrnusBaseDataAsset* Copied = CopyAssetIfMissing(SourceAsset, DestinationPath);
        if (Copied)
        {
            LoadedAssets.Add(SourceAsset->GetClass(), Copied);
        }
    }

    // Ensure RuntimeRegistry exists before loading
    const FString RegistryPath = DestinationDataAssetPath + TEXT("/") + RuntimeRegistryName;
    UCavrnusBaseDataAsset* RegistryAsset = EnsureAssetByClass(UCavrnusDataAssetRegistry::StaticClass(), RegistryPath);
    if (RegistryAsset)
    {
        LoadedAssets.Add(RegistryAsset->GetClass(), RegistryAsset);
    }

    BuildOrUpdateRegistry();
#endif

    // Load RuntimeRegistry from ensured path
    TSoftObjectPtr<UCavrnusDataAssetRegistry> RegistryRef(
        FSoftObjectPath(DestinationDataAssetPath + TEXT("/") + RuntimeRegistryName + TEXT(".") + RuntimeRegistryName)
    );

    if (UCavrnusDataAssetRegistry* Registry = RegistryRef.LoadSynchronous())
    {
        InitializeFromRegistry(Registry);
    }
    else
    {
        UE_LOG(LogCavrnusConnector, Warning, TEXT("CavrnusDataAssetRegistry not found at %s"),
            *RegistryRef.ToSoftObjectPath().ToString());
        return;
    }

    UE_LOG(LogCavrnusConnector, Display, TEXT("DataAssetManager Initialize complete. LoadedAssets.Num() = %d"), LoadedAssets.Num());
    ScanAndPurgeTransientAssets();
}




#if WITH_EDITOR
void UCavrnusDataAssetManager::BuildOrUpdateRegistry()
{
    // Build soft list from discovery
    DiscoveredAssets.Empty();

    // Also add explicitly registered assets (from AssetPaths) as soft references
    for (const auto& Pair : LoadedAssets)
    {
        if (UCavrnusBaseDataAsset* Asset = Pair.Value)
        {
            DiscoveredAssets.Add(TSoftObjectPtr<UCavrnusBaseDataAsset>(Asset));
        }
    }

    // Load or create registry asset
    const FString RegistryObjectPath = DestinationDataAssetPath + TEXT("/") + RuntimeRegistryName + TEXT(".") + RuntimeRegistryName;
    UCavrnusDataAssetRegistry* Registry = LoadObject<UCavrnusDataAssetRegistry>(nullptr, *RegistryObjectPath);

    if (!Registry)
    {
        FAssetToolsModule& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
        Registry = Cast<UCavrnusDataAssetRegistry>(AssetTools.Get().CreateAsset(
            RuntimeRegistryName,
            DestinationDataAssetPath,
            UCavrnusDataAssetRegistry::StaticClass(),
            nullptr));
    }

    if (!Registry)
    {
        UE_LOG(LogCavrnusConnector, Error, TEXT("Failed to create/load CavrnusDataAssetRegistry"));
        return;
    }

    // Deduplicate paths (optional but nice)
    {
        TSet<FSoftObjectPath> Seen;
        TArray<TSoftObjectPtr<UCavrnusBaseDataAsset>> Unique;
        Unique.Reserve(DiscoveredAssets.Num());
        for (const auto& Ref : DiscoveredAssets)
        {
            const FSoftObjectPath Path = Ref.ToSoftObjectPath();
            if (!Seen.Contains(Path))
            {
                Seen.Add(Path);
                Unique.Add(Ref);
            }
        }
        Registry->ManagedAssets = MoveTemp(Unique);
    }

    Registry->MarkPackageDirty();

    // Save package
    UPackage* Package = Registry->GetOutermost();
    const FString PackageFilename = FPackageName::LongPackageNameToFilename(
        Package->GetName(), FPackageName::GetAssetPackageExtension());

    SavePackageVersionSafe(Package, Registry, PackageFilename);
}

#endif


TArray<TSoftObjectPtr<UCavrnusBaseDataAsset>> UCavrnusDataAssetManager::GetSourceAssets(const FString& AssetPath) const
{

    TArray<TSoftObjectPtr<UCavrnusBaseDataAsset>> TheseAssets;
#if WITH_EDITOR
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    AssetRegistryModule.Get().ScanPathsSynchronous({ *AssetPath }, true);

    TArray<FAssetData> AssetDataList;
    AssetRegistryModule.Get().GetAssetsByPath(*AssetPath, AssetDataList, /*bRecursive=*/true);

    for (const FAssetData& AssetData : AssetDataList)
    {
        if (AssetData.GetClass()->IsChildOf(UCavrnusBaseDataAsset::StaticClass()))
        {
            // Build a soft reference from the assets object path
            TSoftObjectPtr<UCavrnusBaseDataAsset> SoftRef(AssetData.ToSoftObjectPath());
            TheseAssets.Add(SoftRef);
        }
    }
#endif
    return TheseAssets;
}

void UCavrnusDataAssetManager::LoadRuntimeAssets()
{
    FSoftObjectPath RegistryPath(DestinationDataAssetPath + "/"+RuntimeRegistryName+"."+RuntimeRegistryName);
    TSoftObjectPtr<UCavrnusDataAssetRegistry> RegistryRef(RegistryPath);

    RegistryRef.LoadSynchronous(); // Or async with StreamableManager

    for (auto& AssetRef : RegistryRef->ManagedAssets)
    {
        UCavrnusBaseDataAsset* LoadedAsset = AssetRef.LoadSynchronous();
        // Use your asset
    }
}

void UCavrnusDataAssetManager::InitializeFromRegistry(UCavrnusDataAssetRegistry* Registry)
{
    for (const TSoftObjectPtr<UCavrnusBaseDataAsset>& AssetRef : Registry->ManagedAssets)
    {
        if (UCavrnusBaseDataAsset* Loaded = AssetRef.LoadSynchronous())
        {
            UE_LOG(LogCavrnusConnector, Display, TEXT("Loaded Asset %s"), *Loaded->GetClass()->GetName());
            LoadedAssets.Add(Loaded->GetClass(), Loaded);
        }
        else
        {
			UE_LOG(LogCavrnusConnector, Warning, TEXT("Failed to Load Asset %s"), *AssetRef.ToSoftObjectPath().ToString());
        }
    }
}

UCavrnusBaseDataAsset* UCavrnusDataAssetManager::EnsureAssetByClass(UClass* AssetClass, const FString& AssetPath)
{
#if WITH_EDITOR
    if (!AssetClass || !AssetClass->IsChildOf(UCavrnusBaseDataAsset::StaticClass()))
    {
        UE_LOG(LogTemp, Error, TEXT("EnsureAssetByClass: Invalid class %s"), AssetClass ? *AssetClass->GetName() : TEXT("nullptr"));
        return nullptr;
    }

    const FString AssetName = FPackageName::GetLongPackageAssetName(AssetPath);
    const FString ObjectPath = AssetPath + TEXT(".") + AssetName;

    UObject* Existing = StaticLoadObject(AssetClass, nullptr, *ObjectPath);
    if (UCavrnusBaseDataAsset* ExistingTyped = Cast<UCavrnusBaseDataAsset>(Existing))
    {
        return ExistingTyped;
    }

    UPackage* Package = CreatePackage(*AssetPath);
    Package->FullyLoad();

    UCavrnusBaseDataAsset* NewAsset = NewObject<UCavrnusBaseDataAsset>(
        Package, AssetClass, *AssetName,
        RF_Public | RF_Standalone | RF_Transactional);

    if (!NewAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("EnsureAssetByClass: Failed to create asset %s"), *AssetName);
        return nullptr;
    }

    FAssetRegistryModule::AssetCreated(NewAsset);
    NewAsset->MarkPackageDirty();

    const FString SavePath = FPackageName::LongPackageNameToFilename(AssetPath, FPackageName::GetAssetPackageExtension());
    SavePackageVersionSafe(Package, NewAsset, SavePath);

    UE_LOG(LogTemp, Warning, TEXT("EnsureAssetByClass: Created new asset %s | Class=%s | Package=%s"),
        *NewAsset->GetName(), *NewAsset->GetClass()->GetName(), *NewAsset->GetOutermost()->GetName());

    return NewAsset;
#else
    return nullptr;
#endif
}



bool UCavrnusDataAssetManager::SavePackageVersionSafe(UPackage* Package, UObject* Asset, const FString& PackageFilename)
{
#if WITH_EDITOR
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    SaveArgs.SaveFlags = SAVE_NoError;
    SaveArgs.bForceByteSwapping = false;
    SaveArgs.bWarnOfLongFilename = true;

    FSavePackageResultStruct Result = UPackage::Save(Package, Asset, *PackageFilename, SaveArgs);
    const bool bSuccess = Result.IsSuccessful();
#else
    const bool bSuccess = UPackage::SavePackage(
        Package,
        Asset,
        EObjectFlags::RF_Public | RF_Standalone,
        *PackageFilename,
        GError,
        nullptr,
        false,
        true,
        SAVE_NoError
    );
#endif

    return bSuccess;
#endif
    return false;
}

void UCavrnusDataAssetManager::ScanAndPurgeTransientAssets()
{
#if WITH_EDITOR
    int32 TransientCount = 0;
    int32 PurgedCount = 0;

    for (TObjectIterator<UCavrnusBaseDataAsset> It; It; ++It)
    {
        UCavrnusBaseDataAsset* Asset = *It;
        if (!IsValid(Asset))
            continue;

        UPackage* Package = Asset->GetOutermost();
        if (!Package || Package->HasAnyPackageFlags(EPackageFlags::PKG_TransientFlags))
        {
            ++TransientCount;

            UE_LOG(LogTemp, Warning, TEXT("Transient ghost detected: %s | Class=%s | Outer=%s"),
                *Asset->GetName(),
                *Asset->GetClass()->GetName(),
                *Asset->GetOuter()->GetName());

            // Optional purge
            Asset->ClearFlags(RF_Public | RF_Standalone);
            Asset->MarkAsGarbage();
            ++PurgedCount;
        }
    }

    UE_LOG(LogTemp, Display, TEXT("Scan complete. Transient assets found: %d | Purged: %d"), TransientCount, PurgedCount);
#else
    UE_LOG(LogTemp, Warning, TEXT("ScanAndPurgeTransientAssets is editor-only"));
#endif
}

UCavrnusBaseDataAsset* UCavrnusDataAssetManager::CopyAssetIfMissing(UObject* SourceAsset, const FString& DestinationPath)
{
#if WITH_EDITOR
    if (!SourceAsset || !SourceAsset->IsA(UCavrnusBaseDataAsset::StaticClass()))
    {
        UE_LOG(LogCavrnusConnector, Warning, TEXT("CopyAssetIfMissing: Invalid source asset"));
        return nullptr;
    }

    const FString AssetName = SourceAsset->GetName();
    const FString ObjectPath = DestinationPath + TEXT(".") + AssetName;

    UObject* Existing = StaticLoadObject(SourceAsset->GetClass(), nullptr, *ObjectPath);
    if (UCavrnusBaseDataAsset* ExistingTyped = Cast<UCavrnusBaseDataAsset>(Existing))
    {
        return ExistingTyped;
    }

    UPackage* DestPackage = CreatePackage(*DestinationPath);
    DestPackage->FullyLoad();

    UCavrnusBaseDataAsset* Duplicated = Cast<UCavrnusBaseDataAsset>(
        DuplicateObject(SourceAsset, DestPackage, *AssetName));

    if (!Duplicated)
    {
        UE_LOG(LogCavrnusConnector, Warning, TEXT("CopyAssetIfMissing: Failed to duplicate asset %s"), *AssetName);
        return nullptr;
    }

    FAssetRegistryModule::AssetCreated(Duplicated);
    Duplicated->MarkPackageDirty();

    const FString SavePath = FPackageName::LongPackageNameToFilename(DestinationPath, FPackageName::GetAssetPackageExtension());
    SavePackageVersionSafe(DestPackage, Duplicated, SavePath);

    UE_LOG(LogTemp, Warning, TEXT("CopyAssetIfMissing: Duplicated asset %s | Class=%s | Package=%s"),
        *Duplicated->GetName(), *Duplicated->GetClass()->GetName(), *Duplicated->GetOutermost()->GetName());

    return Duplicated;
#else
    return nullptr;
#endif
}



bool UCavrnusDataAssetManager::CheckUnrealVersionNewerThanEngine(const FString& PackageFilename)
{
    TUniquePtr<FArchive> Ar(IFileManager::Get().CreateFileReader(*PackageFilename));
    if (!Ar || Ar->IsError())
    {
        return false; // Can't read file, assume safe
    }

    // Skip magic number (4 bytes)
    Ar->Seek(4);

    // Read legacy version (unused here)
    int32 LegacyVersion = 0;
    *Ar << LegacyVersion;

    // Read UE version
    int32 UEVersion = 0;
    *Ar << UEVersion;

    const int32 SupportedVersion = VER_LATEST_ENGINE_UE5;

    return UEVersion > SupportedVersion;
}