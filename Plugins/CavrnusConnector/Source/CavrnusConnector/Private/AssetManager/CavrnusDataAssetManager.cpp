#include "AssetManager/CavrnusDataAssetManager.h"
#include "AssetManager/CavrnusMigrationManager.h"
#include "CavrnusConnectorModule.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Serialization/Archive.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "AssetRegistry/AssetRegistryModule.h"

#if WITH_EDITOR
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/Application/SlateApplication.h"
#include "Editor.h"
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
#include "PackageHelperFunctions.h"
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
#include "UObject/SavePackage.h"
#else
#include "PackageTools.h"
#endif
#endif
#endif

void UCavrnusDataAssetManager::Initialize()
{
    Super::Initialize();

    // Step 1: Run pending migrations before any asset work
    MigrationManager = NewObject<UCavrnusMigrationManager>(this);
    MigrationManager->RunPendingMigrations();

#if WITH_EDITOR
    // Step 2: Scan source assets from plugin content
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    AssetRegistryModule.Get().ScanPathsSynchronous({ *SourceDataAssetPath }, true);

    TArray<FAssetData> AssetDataList;
    AssetRegistryModule.Get().GetAssetsByPath(*SourceDataAssetPath, AssetDataList, true);

    // Step 3: For each source asset — load it to get the real derived class,
    // then check if /Game/ copy exists; if missing, copy it
    for (const FAssetData& AssetData : AssetDataList)
    {
        const FString AssetName = AssetData.AssetName.ToString();
        const FString DestinationPath = DestinationDataAssetPath + TEXT("/") + AssetName;

        // Must load the source to get the actual derived UClass
        TSoftObjectPtr<UCavrnusBaseDataAsset> SourceRef(AssetData.ToSoftObjectPath());
        UCavrnusBaseDataAsset* SourceAsset = SourceRef.LoadSynchronous();
        if (!SourceAsset)
        {
            UE_LOG(LogCavrnusConnector, Warning, TEXT("Failed to load source asset %s"), *AssetData.ToSoftObjectPath().ToString());
            continue;
        }

        UClass* AssetClass = SourceAsset->GetClass();
        if (!AssetClass->IsChildOf(UCavrnusBaseDataAsset::StaticClass()))
        {
            UE_LOG(LogCavrnusConnector, Warning, TEXT("Skipping asset %s: not a CavrnusBaseDataAsset subclass"), *AssetName);
            continue;
        }

        // Check if destination already exists (check file on disk first to avoid noisy linker warnings on first run)
        const FString DestObjectPath = DestinationPath + TEXT(".") + AssetName;
        const FString DestPackageFilename = FPackageName::LongPackageNameToFilename(DestinationPath, FPackageName::GetAssetPackageExtension());
        if (FPaths::FileExists(DestPackageFilename))
        {
            UObject* Existing = StaticLoadObject(AssetClass, nullptr, *DestObjectPath);
            if (UCavrnusBaseDataAsset* ExistingTyped = Cast<UCavrnusBaseDataAsset>(Existing))
            {
                LoadedAssets.Add(AssetClass, ExistingTyped);
                AssetPaths.Add(AssetClass, DestObjectPath);
                continue;
            }
        }
        else
        {
            UE_LOG(LogCavrnusConnector, Display, TEXT("Project copy of '%s' not found -- will be created from plugin source"), *AssetName);
        }

        // Copy source asset to destination
        UCavrnusBaseDataAsset* Copied = CopyAssetIfMissing(SourceAsset, DestinationPath);
        if (Copied)
        {
            LoadedAssets.Add(AssetClass, Copied);
            AssetPaths.Add(AssetClass, DestObjectPath);
        }
    }

    // Step 4: Check for version updates and notify
    CheckAndNotifyVersionUpdates();
#else
    // Runtime: Scan /Game/Cavrnus/DataAssets/ via AssetRegistry and load all
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

    TArray<FAssetData> RuntimeAssetDataList;
    AssetRegistryModule.Get().GetAssetsByPath(*DestinationDataAssetPath, RuntimeAssetDataList, true);

    for (const FAssetData& AssetData : RuntimeAssetDataList)
    {
        // FAssetData::GetClass() returns the metadata UClass (e.g. UDataAsset),
        // NOT the loaded object's derived class. Load first, then check.
        UCavrnusBaseDataAsset* Loaded = Cast<UCavrnusBaseDataAsset>(AssetData.GetAsset());
        if (!Loaded)
        {
            continue;
        }

        UClass* AssetClass = Loaded->GetClass();
        LoadedAssets.Add(AssetClass, Loaded);
        AssetPaths.Add(AssetClass, AssetData.ToSoftObjectPath().ToString());
        UE_LOG(LogCavrnusConnector, Display, TEXT("Loaded runtime asset: %s (%s)"), *Loaded->GetName(), *AssetClass->GetName());
    }
#endif

    UE_LOG(LogCavrnusConnector, Verbose, TEXT("DataAssetManager Initialize complete. LoadedAssets.Num() = %d"), LoadedAssets.Num());
}

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
        // FAssetData::GetClass() returns metadata UClass — load to get derived class
        if (UCavrnusBaseDataAsset* Loaded = Cast<UCavrnusBaseDataAsset>(AssetData.GetAsset()))
        {
            TSoftObjectPtr<UCavrnusBaseDataAsset> SoftRef(AssetData.ToSoftObjectPath());
            TheseAssets.Add(SoftRef);
        }
    }
#endif
    return TheseAssets;
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
    const FString PackageFilename = FPackageName::LongPackageNameToFilename(DestinationPath, FPackageName::GetAssetPackageExtension());

    // Check if destination already exists — pure copy-only-if-missing
    UObject* Existing = StaticLoadObject(SourceAsset->GetClass(), nullptr, *ObjectPath);
    if (UCavrnusBaseDataAsset* ExistingTyped = Cast<UCavrnusBaseDataAsset>(Existing))
    {
        return ExistingTyped;
    }

    // Duplicate source asset to destination
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

    SavePackageVersionSafe(DestPackage, Duplicated, PackageFilename);

    UE_LOG(LogCavrnusConnector, Log, TEXT("CopyAssetIfMissing: Duplicated asset %s to %s"), *AssetName, *DestinationPath);

    return Duplicated;
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

bool UCavrnusDataAssetManager::CheckUnrealVersionNewerThanEngine(const FString& PackageFilename)
{
    TUniquePtr<FArchive> Ar(IFileManager::Get().CreateFileReader(*PackageFilename));
    if (!Ar || Ar->IsError())
    {
        return false;
    }

    Ar->Seek(4);

    int32 LegacyVersion = 0;
    *Ar << LegacyVersion;

    int32 UEVersion = 0;
    *Ar << UEVersion;

    const int32 SupportedVersion = VER_LATEST_ENGINE_UE5;
    return UEVersion > SupportedVersion;
}

bool UCavrnusDataAssetManager::CheckUnrealVersionCompatible(const FString& PackageFilename)
{
#if WITH_EDITOR
    TUniquePtr<FArchive> Reader(IFileManager::Get().CreateFileReader(*PackageFilename));
    if (!Reader || Reader->IsError())
    {
        return false;
    }

    FPackageFileSummary Summary;
    *Reader << Summary;
    if (Reader->IsError())
    {
        return false;
    }

    FPackageFileVersion PackageFileVersionUE = Summary.GetFileVersionUE();
    if (PackageFileVersionUE.FileVersionUE5 <= 0 || Summary.TotalHeaderSize <= 0)
    {
        return false;
    }

    const FEngineVersion Current = FEngineVersion::Current();
    const FEngineVersion Saved = Summary.CompatibleWithEngineVersion;

    if (Current.GetMajor() > Saved.GetMajor()) return true;
    if (Current.GetMajor() == Saved.GetMajor())
    {
        if (Current.GetMinor() > Saved.GetMinor()) return true;
        if (Current.GetMinor() == Saved.GetMinor() && Current.GetPatch() >= Saved.GetPatch())
            return true;
    }

    return false;
#else
    return false;
#endif
}

#if WITH_EDITOR
void UCavrnusDataAssetManager::CheckAndNotifyVersionUpdates()
{
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

    TArray<FAssetData> SourceAssetDataList;
    AssetRegistryModule.Get().GetAssetsByPath(*SourceDataAssetPath, SourceAssetDataList, true);

    // Collect all outdated assets
    struct FOutdatedAsset
    {
        UCavrnusBaseDataAsset* Source;
        UCavrnusBaseDataAsset* ProjectCopy;
        FString Name;
        int32 SourceVersion;
        int32 ProjectVersion;
    };
    TArray<FOutdatedAsset> OutdatedAssets;

    for (const FAssetData& SourceData : SourceAssetDataList)
    {
        UCavrnusBaseDataAsset* SourceAsset = Cast<UCavrnusBaseDataAsset>(SourceData.GetAsset());
        if (!SourceAsset)
            continue;

        UClass* AssetClass = SourceAsset->GetClass();
        UCavrnusBaseDataAsset* ProjectCopy = LoadedAssets.FindRef(AssetClass);
        if (!ProjectCopy)
            continue;

        if (SourceAsset->AssetVersion > ProjectCopy->AssetVersion)
        {
            FOutdatedAsset Entry;
            Entry.Source = SourceAsset;
            Entry.ProjectCopy = ProjectCopy;
            Entry.Name = SourceAsset->GetName();
            Entry.SourceVersion = SourceAsset->AssetVersion;
            Entry.ProjectVersion = ProjectCopy->AssetVersion;
            OutdatedAssets.Add(Entry);
        }
    }

    if (OutdatedAssets.Num() == 0)
        return;

    // Build message listing all outdated assets
    FString AssetList;
    for (const FOutdatedAsset& Entry : OutdatedAssets)
    {
        AssetList += FString::Printf(TEXT("  - %s (v%d -> v%d)\n"), *Entry.Name, Entry.ProjectVersion, Entry.SourceVersion);
    }

    const FString SummaryMessage = FString::Printf(
        TEXT("%d Cavrnus data asset(s) have updates available:\n%s"),
        OutdatedAssets.Num(), *AssetList);

    UE_LOG(LogCavrnusConnector, Warning, TEXT("%s"), *SummaryMessage);

    // Build the custom dialog
    TSharedPtr<SWindow> DialogWindow = SNew(SWindow)
        .Title(FText::FromString(TEXT("Cavrnus Data Asset Updates")))
        .SizingRule(ESizingRule::Autosized)
        .AutoCenter(EAutoCenter::PreferredWorkArea)
        .SupportsMaximize(false)
        .SupportsMinimize(false);

    // Result tracking
    enum class EUpdateChoice { None, UpdateWithBackup, UpdateNoBackup, Dismiss };
    TSharedPtr<EUpdateChoice> UserChoice = MakeShared<EUpdateChoice>(EUpdateChoice::None);
    TSharedRef<SCheckBox> BackupCheckbox = SNew(SCheckBox)
        .IsChecked(ECheckBoxState::Checked);

    TWeakPtr<SWindow> WeakWindow = DialogWindow;

    DialogWindow->SetContent(
        SNew(SBox)
        .MinDesiredWidth(450)
        .Padding(FMargin(16))
        [
            SNew(SVerticalBox)

            // Header
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 8)
            [
                SNew(STextBlock)
                .Text(FText::FromString(FString::Printf(TEXT("%d data asset(s) have updates available:"), OutdatedAssets.Num())))
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
            ]

            // Asset list
            + SVerticalBox::Slot()
            .AutoHeight()
            .MaxHeight(200)
            .Padding(0, 0, 0, 12)
            [
                SNew(SScrollBox)
                + SScrollBox::Slot()
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(AssetList))
                    .Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
                ]
            ]

            // Backup checkbox
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 16)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                [
                    BackupCheckbox
                ]
                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                .Padding(6, 0, 0, 0)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("Create backups before updating (saved to /Game/Cavrnus/DataAssetBackups/)")))
                    .Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
                ]
            ]

            // Buttons
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .FillWidth(1.0f)
                [
                    SNew(SButton)
                    .HAlign(HAlign_Center)
                    .Text(FText::FromString(TEXT("Update All")))
                    .OnClicked_Lambda([UserChoice, WeakWindow]()
                    {
                        *UserChoice = EUpdateChoice::UpdateWithBackup;
                        if (TSharedPtr<SWindow> Win = WeakWindow.Pin())
                            Win->RequestDestroyWindow();
                        return FReply::Handled();
                    })
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(8, 0, 0, 0)
                [
                    SNew(SButton)
                    .HAlign(HAlign_Center)
                    .Text(FText::FromString(TEXT("Skip")))
                    .OnClicked_Lambda([UserChoice, WeakWindow]()
                    {
                        *UserChoice = EUpdateChoice::Dismiss;
                        if (TSharedPtr<SWindow> Win = WeakWindow.Pin())
                            Win->RequestDestroyWindow();
                        return FReply::Handled();
                    })
                ]
            ]
        ]
    );

    // Show as modal
    GEditor->EditorAddModalWindow(DialogWindow.ToSharedRef());

    // Process result
    if (*UserChoice == EUpdateChoice::UpdateWithBackup || *UserChoice == EUpdateChoice::UpdateNoBackup)
    {
        const bool bDoBackup = BackupCheckbox->IsChecked();

        for (const FOutdatedAsset& Entry : OutdatedAssets)
        {
            if (IsValid(Entry.Source) && IsValid(Entry.ProjectCopy))
            {
                if (bDoBackup)
                    BackupDataAsset(Entry.ProjectCopy, Entry.ProjectVersion);
                UpdateDataAsset(Entry.Source, Entry.ProjectCopy);
            }
        }

        FNotificationInfo DoneInfo(FText::FromString(
            bDoBackup
                ? TEXT("Cavrnus data assets updated (backups created)")
                : TEXT("Cavrnus data assets updated")));
        DoneInfo.ExpireDuration = 5.0f;
        FSlateNotificationManager::Get().AddNotification(DoneInfo);
    }
}

void UCavrnusDataAssetManager::UpdateDataAsset(UCavrnusBaseDataAsset* SourceAsset, UCavrnusBaseDataAsset* ProjectCopy)
{
    if (!SourceAsset || !ProjectCopy)
        return;

    const FString AssetName = ProjectCopy->GetName();
    const FString DestPath = DestinationDataAssetPath / AssetName;
    const FString PackageFilename = FPackageName::LongPackageNameToFilename(DestPath, FPackageName::GetAssetPackageExtension());

    // Delete the old project copy's package
    UPackage* OldPackage = ProjectCopy->GetOutermost();

    // Duplicate source to destination, replacing the old asset
    UPackage* DestPackage = CreatePackage(*DestPath);
    DestPackage->FullyLoad();

    UCavrnusBaseDataAsset* NewCopy = Cast<UCavrnusBaseDataAsset>(
        DuplicateObject(SourceAsset, DestPackage, *AssetName));

    if (!NewCopy)
    {
        UE_LOG(LogCavrnusConnector, Error, TEXT("UpdateDataAsset: Failed to duplicate %s"), *AssetName);
        return;
    }

    FAssetRegistryModule::AssetCreated(NewCopy);
    NewCopy->MarkPackageDirty();
    SavePackageVersionSafe(DestPackage, NewCopy, PackageFilename);

    // Update our loaded reference
    UClass* AssetClass = SourceAsset->GetClass();
    LoadedAssets.Add(AssetClass, NewCopy);

    UE_LOG(LogCavrnusConnector, Log, TEXT("UpdateDataAsset: Updated %s to v%d"), *AssetName, SourceAsset->AssetVersion);
}

void UCavrnusDataAssetManager::BackupDataAsset(UCavrnusBaseDataAsset* Asset, int32 Version)
{
    if (!Asset)
        return;

    const FString AssetName = Asset->GetName();
    const FString VersionFolder = FString::Printf(TEXT("%s/v%d"), *BackupDataAssetPath, Version);
    const FString BackupPath = VersionFolder / AssetName;
    const FString PackageFilename = FPackageName::LongPackageNameToFilename(BackupPath, FPackageName::GetAssetPackageExtension());

    // Check if backup already exists
    if (FPaths::FileExists(PackageFilename))
    {
        UE_LOG(LogCavrnusConnector, Verbose, TEXT("BackupDataAsset: Backup already exists for %s v%d"), *AssetName, Version);
        return;
    }

    UPackage* BackupPackage = CreatePackage(*BackupPath);
    BackupPackage->FullyLoad();

    UCavrnusBaseDataAsset* BackupCopy = Cast<UCavrnusBaseDataAsset>(
        DuplicateObject(Asset, BackupPackage, *AssetName));

    if (!BackupCopy)
    {
        UE_LOG(LogCavrnusConnector, Warning, TEXT("BackupDataAsset: Failed to backup %s"), *AssetName);
        return;
    }

    FAssetRegistryModule::AssetCreated(BackupCopy);
    BackupCopy->MarkPackageDirty();
    SavePackageVersionSafe(BackupPackage, BackupCopy, PackageFilename);

    UE_LOG(LogCavrnusConnector, Log, TEXT("BackupDataAsset: Backed up %s v%d to %s"), *AssetName, Version, *BackupPath);
}
#endif
