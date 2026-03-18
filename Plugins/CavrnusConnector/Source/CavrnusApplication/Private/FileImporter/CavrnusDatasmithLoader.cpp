#include "FileImporter/CavrnusDatasmithLoader.h"
#include "FileImporter/DatasmithFileImporter.h"
#include "Managers/SpawnedObjects/SpawnedObjectsManager.h"
#include "Core/Subsystems/CavrnusSubsystem.h"
#include "Core/Contexts/CavrnusRuntimeContext.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "FileImporter/DatasmithRepairProcessor.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "CavrnusFunctionLibrary.h"

/*
===============================================================================
 Datasmith Import Pipeline (UCavrnusDatasmithLoader)

 1. Download & Stage
    - ResolveFileIdToLocalPathAsync
    - Fetch file by FileID
    - If .zip → unzip into Saved/<CacheFolder>/<FileId>/...
    - Return resolved .udatasmith scene path + list of extracted files

 2. Mount & Sync
    - MapCacheTopLevel(FileId, ExtractedFiles)
      • Mount per-FileId subdirectories under /CavrnusCache/<FileId>/...
      • For each extracted Twinmotion asset:
          - If authored path is /Game/Twinmotion/... and package not present
            → copy into Saved/TwinmotionCommon/...
      • Register global mount /Game/Twinmotion/ → Saved/TwinmotionCommon/Content/Twinmotion

 3. Metadata Extraction
    - UDatasmithFileLibrary::ExtractMeshAndMaterialMetadata
    - Parse .udatasmith XML
    - Build MeshSlotMap (mesh GUID → material slot IDs)
    - Build MaterialMap (material GUID → rewritten path)

 4. Path Normalization
    - NormalizeExtractedDependencies(FileId, BaseName, ExtractedFiles, MaterialMap, MeshSlotMap)
      • Skip /Game/Twinmotion/_common (global mount resolves)
      • Rewrite other /Game/... paths into /CavrnusCache/<FileId>/...

 5. Import
    - ActiveImporter->ImportFile(ResolvedPath, Settings, StatusDelegate, CompleteDelegate)
    - Spawns ADatasmithRuntimeActor
    - Loads scene into world
    - Broadcasts status updates

 6. Repair
    - UDatasmithRepairProcessor::RepairActorMaterials
    - Walk meshes in imported actor
    - For empty/placeholder slots:
        • Look up intended material GUID
        • Load material via normalized path
        • Set material on mesh slot
        • Broadcast repair status

===============================================================================
*/


void UCavrnusDatasmithLoader::DoLoadInternal(const FCavrnusSpawnedObject& ObjectData, UWorld* World)
{
    WorldContext = World;
    ContainerName = ObjectData.PropertiesContainerName;

    // Subscribe to property updates for <Container>/DatasmithFile
    UCavrnusBinding* PropertyBinding = UCavrnusFunctionLibrary::BindGenericPropertyValue(
        ObjectData.SpaceConnection,
        ContainerName,
        FileInfoPropertyName,
        [this](const Cavrnus::FPropertyValue& Value, const FString& Container, const FString& PropertyName)
        {
            HandlePropertyChanged(Value, Container, PropertyName);
        });

}

void UCavrnusDatasmithLoader::HandlePropertyChanged(const Cavrnus::FPropertyValue& Value, const FString& Container, const FString& PropertyName)
{
    // Parse JSON or struct: { "FileID": "...", "FileName": "..." }
    FString FileID, FileName;
    if (Value.PropType == Cavrnus::FPropertyValue::PropertyType::String)
    {
        if (ParseFileInfoJson(Value.StringValue, FileID, FileName))
        {
            BeginImport(FileID, FileName);
        }
    }
    else
    {
		UE_LOG(LogTemp, Warning, TEXT("DatasmithLoader: Unsupported property type for %s/%s"), *Container, *PropertyName);
    }
}

void UCavrnusDatasmithLoader::OnImporterStatus(const FCavrnusImportStatus& Status)
{
    FCavrnusImportStatus Copy = Status;
    Copy.FileKey = FileIDFolder;
    BroadcastStatus(Copy);
}

void UCavrnusDatasmithLoader::OnImporterComplete(const FCavrnusImportStatus& FinalStatus)
{
    FCavrnusImportStatus CopyStatus = FinalStatus;
    CopyStatus.FileKey = FileIDFolder;
    BroadcastComplete(CopyStatus);

    if (UCavrnusConnectorSettings::Get()->bEnableDatasmithMaterialRepair)
    {
        ADatasmithRuntimeActor* TargetActor = ActiveImporter->TargetActor.Get();
        if (TargetActor)
        {
            TFunction<void(const FCavrnusImportStatus&)> StatusReporter =
                [this, TargetActor](const FCavrnusImportStatus& Status)
            {
                BroadcastStatus(Status);

                if (LastImportedFileType == EDatasmithRuntimeFileType::Twinmotion)
                {
                    // GUID-based repair first
                    UDatasmithRepairProcessor::RepairActorMaterials(TargetActor, MaterialMap, MeshSlotMap,
                        [this](const FCavrnusImportStatus& Status) { BroadcastStatus(Status); });

                    UDatasmithFileImporter::ProcessTwinmotionDatasmithMaterials(LastImportedFilePath, TargetActor);
                }
            };
        }
    }
    ActiveImporter = nullptr;
}

// Walk the extracted Content/Twinmotion tree and rewrite any /Game/... references
void UCavrnusDatasmithLoader::NormalizeExtractedDependencies(
    const FString& UniqueId,
    const FString& BaseName,
    const TArray<FString>& ExtractedFiles,
    TMap<FString, FDatasmithMaterialMetadata>& InMaterialMap,
    TMap<FString, FDatasmithMeshSlotMetadata>& InMeshSlotMap)
{
    // --- Normalize material metadata ---
    for (auto& Pair : InMaterialMap)
    {
        FDatasmithMaterialMetadata& Meta = Pair.Value;

        if (!Meta.PathName.StartsWith(TEXT("/Game/Twinmotion/_common")) &&
            Meta.PathName.StartsWith(TEXT("/Game/")))
        {
            Meta.PathName = UDatasmithFileLibrary::RewritePath(Meta.PathName, UniqueId, BaseName);
        }

        for (FString& DepPath : Meta.TextureRefs)
        {
            if (!DepPath.StartsWith(TEXT("/Game/Twinmotion/_common")) &&
                DepPath.StartsWith(TEXT("/Game/")))
            {
                DepPath = UDatasmithFileLibrary::RewritePath(DepPath, UniqueId, BaseName);
            }
        }
    }

    // --- Normalize mesh slot metadata ---
    for (auto& Pair : InMeshSlotMap)
    {
        FDatasmithMeshSlotMetadata& MeshMeta = Pair.Value;

        for (FString& MatPath : MeshMeta.MaterialPaths)
        {
            if (!MatPath.StartsWith(TEXT("/Game/Twinmotion/_common")) &&
                MatPath.StartsWith(TEXT("/Game/")))
            {
                MatPath = UDatasmithFileLibrary::RewritePath(MatPath, UniqueId, BaseName);
            }
        }
    }
}


void UCavrnusDatasmithLoader::MapCacheTopLevel(const FString& UniqueId, const TArray<FString>& ExtractedFiles)
{
    const FString CacheFolder = UCavrnusSubsystem::Get()->RuntimeContext
        ->Get<USpawnedObjectsManager>()->GetCacheFolder();
    const FString PhysicalRoot = FPaths::Combine(FPaths::ProjectSavedDir(), CacheFolder, UniqueId);

    // --- Mount scene-specific subdirectories ---
    TArray<FString> Subdirs;
    IFileManager::Get().FindFiles(Subdirs, *PhysicalRoot, false, true);

    for (const FString& Subdir : Subdirs)
    {
        const FString PhysicalPath = FPaths::Combine(PhysicalRoot, Subdir);
        const FString VirtualMount = FString::Printf(TEXT("/%s/%s/%s/"),
            *CacheFolder, *UniqueId, *Subdir);

        FPackageName::RegisterMountPoint(VirtualMount, PhysicalPath);
        UE_LOG(LogTemp, Log, TEXT("Mounted %s -> %s"), *VirtualMount, *PhysicalPath);
    }

    // --- Sync extracted Twinmotion assets into global /Game/Twinmotion tree ---
    const FString GlobalRoot = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("TwinmotionCommon"));

    for (const FString& File : ExtractedFiles)
    {
        if (!FPaths::GetExtension(File).Equals(TEXT("uasset"), ESearchCase::IgnoreCase))
            continue;

        FString RelPath = File;
        RelPath.RemoveFromStart(PhysicalRoot);
        RelPath.RemoveFromStart(TEXT("/"));

        if (!RelPath.StartsWith(TEXT("Content/Twinmotion")))
            continue;

        const FString VirtualPath = FString::Printf(TEXT("/Game/%s"), *RelPath);

        // Skip if already present in project
        if (FPackageName::DoesPackageExist(VirtualPath))
        {
            UE_LOG(LogTemp, Log, TEXT("Package %s already exists, skipping copy"), *VirtualPath);
            continue;
        }

        FString GlobalDest = FPaths::Combine(GlobalRoot, RelPath);
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(GlobalDest), true);
        IFileManager::Get().Copy(*GlobalDest, *File, /*Replace=*/true);
        UE_LOG(LogTemp, Log, TEXT("Synced %s -> %s"), *File, *GlobalDest);
    }

    // --- Mount global TwinmotionCommon once ---
    const FString VirtualCommonMount = TEXT("/Game/Twinmotion/");
    FPackageName::RegisterMountPoint(VirtualCommonMount, GlobalRoot / TEXT("Content/Twinmotion"));
    UE_LOG(LogTemp, Log, TEXT("Mounted shared Twinmotion -> %s"), *(GlobalRoot / TEXT("Content/Twinmotion")));
}




void UCavrnusDatasmithLoader::BeginImport(const FString& FileID, const FString& FileName)
{
	const FString CacheFolder = UCavrnusSubsystem::Get()->RuntimeContext->Get<USpawnedObjectsManager>()->GetCacheFolder();
    const FString TempFolder = FPaths::ProjectSavedDir() / CacheFolder;
    FString BaseName = FPaths::GetBaseFilename(FileName);

    FileIDFolder = FPaths::Combine(TempFolder, FileID);


    ResolveFileIdToLocalPathAsync(FileID, FileName, TempFolder,
        [this, FileID, BaseName](bool bSuccess, const FString& ResolvedPath, const TArray<FString>& AllExtracted, const FString& ErrorMsg)
        {
            for (const FString& P : AllExtracted)
            {
                UE_LOG(LogTemp, Log, TEXT("  Extracted: %s"), *P);
            }

            if (!IFileManager::Get().FileExists(*ResolvedPath))
            {
                UE_LOG(LogTemp, Warning, TEXT("ResolvedPath does not exist on disk: %s"), *ResolvedPath);
            }

            if (!bSuccess)
            {
                FCavrnusImportStatus Fail;
                Fail.Progress = 0.f;
                Fail.FileKey = FileIDFolder;
                Fail.StatusMessage = ErrorMsg;
                Fail.SecondaryMessage = ResolvedPath;
                Fail.bSuccess = false;
                BroadcastComplete(Fail);
                return;
            }

            // Create importer and keep it alive
            ActiveImporter = NewObject<UDatasmithFileImporter>(WorldContext.Get());
            if (!ActiveImporter)
            {
                FCavrnusImportStatus Fail;
                Fail.Progress = 0.f;
                Fail.FileKey = FileIDFolder;
                Fail.StatusMessage = TEXT("Failed to create importer");
                Fail.SecondaryMessage = ResolvedPath;
                Fail.bSuccess = false;
                BroadcastComplete(Fail);
                return;
            }

            LastImportedFilePath = ResolvedPath;

            FOnImportStatusUpdateDynamic StatusDelegate;
            StatusDelegate.BindUFunction(this, FName("OnImporterStatus"));

            FOnImportCompleteDynamic CompleteDelegate;
            CompleteDelegate.BindUFunction(this, FName("OnImporterComplete"));

            TArray<FString> ExtractedFiles = AllExtracted;
            MapCacheTopLevel(FileID, ExtractedFiles);

            // Okay, we should be fully unzipped by here
            FString ErrorMessage;
            LastImportedFileType = UDatasmithFileLibrary::GetFileTypeFromDatasmithFile(ResolvedPath, ErrorMessage);
            if (LastImportedFileType == EDatasmithRuntimeFileType::Unknown)
            {
                UE_LOG(LogTemp, Warning, TEXT("File type detection failed: %s"), *ErrorMessage);
            }

            if (!UDatasmithFileLibrary::ExtractMeshAndMaterialMetadata(ResolvedPath, FileID, BaseName,
                                                                        MeshSlotMap, MaterialMap, ErrorMessage))
            {
                UE_LOG(LogTemp, Warning, TEXT("No mesh/material metadata extracted from %s"), *ResolvedPath);
            }
 
            NormalizeExtractedDependencies(FileID, BaseName, ExtractedFiles, MaterialMap, MeshSlotMap);


            FCavrnusImportSettings Settings;
            // Ensure ImportFile runs on GameThread
            ActiveImporter->ImportFile(ResolvedPath, Settings, StatusDelegate, CompleteDelegate);
        });
}

void UCavrnusDatasmithLoader::BeginDestroy()
{
    if (ActiveImporter)
    {
        ActiveImporter->OnImportStatusUpdate.Clear();
        ActiveImporter->OnImportComplete.Clear();
        ActiveImporter = nullptr;
    }
    Super::BeginDestroy();
}

