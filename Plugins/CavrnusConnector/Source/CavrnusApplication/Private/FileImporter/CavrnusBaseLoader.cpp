// CavrnusBaseLoader.cpp
// Refactored: single GameThread pump, worker tasks return POD results, clear lifetime and cancellation handling.

#include "FileImporter/CavrnusBaseLoader.h"
#include "FileImporter/UnzipWorker.h"
#include "CavrnusFunctionLibrary.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "ZipUtilities/ZipLibrary.h"
#include "Async/Async.h"
#include "Types/CavrnusRemoteContent.h"
#include "Containers/Set.h"
#include "Templates/SharedPointer.h"
#include "HAL/PlatformTime.h"
#include "Templates/Atomic.h"
#include <atomic>
#include "unzip.h"
#include "Async/TaskGraphInterfaces.h"
#include "Containers/Queue.h"


/*
* 
* There's more than a couple moving parts in this
* Baseclasses are the CavrnusBaseLoader and CavrnusFileImporter.
* Specific loader/importer types inherit from those classes.
* The CavnusBaseLoader handles most of the task of getting the file from the cloud and unzipping it
*     and making it ready for the CavrnusFileImporter
* From a UI perspective, we're currently tying the UI to the Toast messages which allow the code to provide continual status updates of the load process
* There are a few different ways users may zip up their files.  The original CVT plugin would zip up the contents of the datasmith directory individually and then zip that up,
* but that's more cumbersome than needed and the code now supports one zip with the object hierarchy underneath it.  It may be necessary to support extension of that.
* The DatasmithFileImporter and CavrnusDatasmithLoader can be used as examples of subclasses that use the BaseLoader and FileImporter.
* Also, since portions of the download/unzip/import process are asynchronous, the CavrnusBaseLoader implements a Pump and Queue so that Worker thread data can be queued
* up for the Game thread to consume at its convenience.  Without these AsyncTasks being sent to the game thread, UI updates to the Gamethread are frequently missed.

Async import workflow (current architecture)

[GameThread] UCavrnusBaseLoader::StartLoad
    - Initializes loader state
    - Kicks off ResolveFileIdToLocalPathAsync

[Download Worker] UCavrnusFunctionLibrary::FetchFileByIdToDisk (ThreadPool)
    - Streams progress via BroadcastStatus("Downloading ...")
    - On success: produces FDownloadResult
    - On failure: produces FDownloadResult with error

[Pump] FCavrnusPump::PushResult(FDownloadResult)
    - Queues result and schedules Drain() on GameThread

[GameThread Continuation: Download]
    - If not a zip → move file into FileIDFolder → OnComplete(success)
    - If zip → call StageArchiveIntoFileIDFolder for top-level staging
    - Registers further unzip tasks into ProcessExtractionQueue

[Unzip Worker] Worker_UnzipAll (ThreadPool, in UnzipWorker.cpp)
    - Extracts files from zip
    - Calls progress callback → BroadcastStatus("Extracting <file>")
    - Produces FUnzipResult (list of files, error)

[Pump] FCavrnusPump::PushResult(FUnzipResult)
    - Queues result and schedules Drain() on GameThread

[GameThread Continuation: Unzip]
    - If failure → BroadcastStatus("Partial unzip failure"), continue
    - If success → BroadcastStatus("Unzip Complete: N files")
    - Merges extracted files into AllExtracted
    - Adds nested zips back into Queue
    - Recursively calls ProcessExtractionQueue until all zips processed

[Final Resolution]
    - CleanAndFlattenExtraction chooses the .udatasmith scene file
    - OnComplete(true/false, ScenePath, ExtractedFiles, Error)

[Broadcast]
    - BroadcastComplete(FinalStatus) notifies both Blueprint and native delegates
    - SpawnedObjectsManager shows progress toasts and info toasts
*/


// Increment/decrement:
void UCavrnusBaseLoader::IncrementInflight()
{
    int32 Count = ++InflightOps;
    if (!bRootedForInflight)
    {
        AddToRoot(); // Investigate removal as well as matching in DecrementInflight()
        bRootedForInflight = true;
    }
    LOG_CAVRNUS_VERBOSE("[%s] IncrementInflight → %d", *GetNameSafe(this), Count);
}

void UCavrnusBaseLoader::DecrementInflight()
{
    int32 Count = --InflightOps;
    LOG_CAVRNUS_VERBOSE("[%s] DecrementInflight → %d", *GetNameSafe(this), Count);
    if (Count == 0 && bRootedForInflight)
    {
        RemoveFromRoot();
        bRootedForInflight = false;
    }
}

void UCavrnusBaseLoader::StartLoad(const FCavrnusSpawnedObject& ObjectData, UWorld* World)
{
    bImportCancelled = false;

    // Explicit construction is required in UE 5.6
    WorldContext = TWeakObjectPtr<UWorld>(World);

    DoLoadInternal(ObjectData, World);
}


void UCavrnusBaseLoader::CancelLoad()
{
    bImportCancelled = true;
}

//////////////////////////////////////////////////////////////////////////
// Delegate safety wrappers (GameThread marshalling, weak-this guarded)

void UCavrnusBaseLoader::BroadcastStatus(const FCavrnusImportStatus& Status)
{
    FCavrnusImportStatus StatusCopy = Status;
    StatusCopy.FileKey = FileIDFolder; // Inject from member

    if (IsInGameThread())
    {
        OnStatusUpdateBP.Broadcast(StatusCopy);
        OnStatusUpdateNative.Broadcast(StatusCopy);
        return;
    }

    TWeakObjectPtr<UCavrnusBaseLoader> WeakThis(this);
    AsyncTask(ENamedThreads::GameThread, [WeakThis, StatusCopy]()
        {
            if (UCavrnusBaseLoader* Self = WeakThis.Get())
            {
                Self->OnStatusUpdateBP.Broadcast(StatusCopy);
                Self->OnStatusUpdateNative.Broadcast(StatusCopy);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("BroadcastStatus: loader expired before GameThread dispatch"));
            }
        });
}

void UCavrnusBaseLoader::BroadcastComplete(const FCavrnusImportStatus& FinalStatus)
{
    FCavrnusImportStatus StatusCopy = FinalStatus;
    StatusCopy.FileKey = FileIDFolder;

    if (IsInGameThread())
    {
        OnCompleteBP.Broadcast(StatusCopy);
        OnCompleteNative.Broadcast(StatusCopy);
        FinalizeLoad();
        return;
    }

    TWeakObjectPtr<UCavrnusBaseLoader> WeakThis(this);
    AsyncTask(ENamedThreads::GameThread, [WeakThis, StatusCopy]()
        {
            if (UCavrnusBaseLoader* Self = WeakThis.Get())
            {
                Self->OnCompleteBP.Broadcast(StatusCopy);
                Self->OnCompleteNative.Broadcast(StatusCopy);
                Self->FinalizeLoad();
            }
        });
}

void UCavrnusBaseLoader::ReportContentLoaded(const FString& Path)
{
    if (IsInGameThread())
    {
        OnContentLoadedNative.ExecuteIfBound(Path);
        OnContentLoadedBP.Broadcast(Path);
        return;
    }

    TWeakObjectPtr<UCavrnusBaseLoader> WeakThis(this);
    const FString PathCopy = Path;
    AsyncTask(ENamedThreads::GameThread, [WeakThis, PathCopy]()
        {
            if (UCavrnusBaseLoader* Self = WeakThis.Get())
            {
                Self->OnContentLoadedNative.ExecuteIfBound(PathCopy);
                Self->OnContentLoadedBP.Broadcast(PathCopy);
            }
        });
}

void UCavrnusBaseLoader::ReportFailure(const FString& Error)
{
    if (IsInGameThread())
    {
        OnFailureNative.ExecuteIfBound(Error);
        OnFailureBP.Broadcast(Error);
        return;
    }

    TWeakObjectPtr<UCavrnusBaseLoader> WeakThis(this);
    const FString ErrorCopy = Error;
    AsyncTask(ENamedThreads::GameThread, [WeakThis, ErrorCopy]()
        {
            if (UCavrnusBaseLoader* Self = WeakThis.Get())
            {
                Self->OnFailureNative.ExecuteIfBound(ErrorCopy);
                Self->OnFailureBP.Broadcast(ErrorCopy);
            }
        });
}

void UCavrnusBaseLoader::FinalizeLoad()
{
    // no-op by default
}

bool UCavrnusBaseLoader::CanHandleId_Implementation(const FString& WellKnownObjectId) const
{
    return false;
}

bool UCavrnusBaseLoader::ParseFileInfoJson(const FString& JsonString, FString& OutFileID, FString& OutFileName) const
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        if (JsonObject->HasField(TEXT("FileID")) && JsonObject->HasField(TEXT("FileName")))
        {
            OutFileID = JsonObject->GetStringField(TEXT("FileID"));
            OutFileName = JsonObject->GetStringField(TEXT("FileName"));
            return true;
        }
    }

    UE_LOG(LogTemp, Error, TEXT("Failed to parse FileInfo JSON: %s"), *JsonString);
    return false;
}

void UCavrnusBaseLoader::BeginDestroy()
{
    CancelLoad();
    LOG_CAVRNUS_VERBOSE("BeginDestroy: PumpInstance valid=%d", PumpInstance.IsValid());

    if (InflightOps.load() > 0)
    {
        LOG_CAVRNUS_VERBOSE("%s::BeginDestroy: %d inflight ops remain",
            *GetNameSafe(this), (int)InflightOps.load());
    }
    else if (PumpInstance)
    {
        PumpInstance.Reset();
    }

    Super::BeginDestroy();
}
void UCavrnusBaseLoader::FinishDestroy()
{
    // Now it’s actually being purged; ensure inflight is zero.
    if (InflightOps.load() == 0 && PumpInstance)
    {
        PumpInstance.Reset();
    }
    Super::FinishDestroy();
}

FString UCavrnusBaseLoader::NormalizeFullPath(const FString& In)
{
    FString Out = FPaths::ConvertRelativePathToFull(In);
    FPaths::NormalizeFilename(Out);
    return Out;
}


// Inputs:
//  - FileIDFolder: CacheRoot/<FileID>/
//  - TopLevelZip:  .../Cars.zip
//  - DesiredSceneName: "Cars.udatasmith"
// Produces:
//  - FileIDFolder/Cars.udatasmith
//  - FileIDFolder/Cars_Assets/...

bool UCavrnusBaseLoader::StageArchiveIntoFileIDFolder(
    const FString& TopLevelZip,
    const FString& DesiredSceneName, // "Cars.udatasmith"
    TArray<FString>& OutExtractedFiles,
    FString& OutResolvedScenePath,
    FString& OutError)
{
    IFileManager& FM = IFileManager::Get();
    FM.MakeDirectory(*FileIDFolder, true);

    auto ReportUnzipProgress = [this, DesiredSceneName](const FString& FileName, float Progress)
    {
        AsyncTask(ENamedThreads::GameThread, [this, DesiredSceneName, FileName, Progress]()
        {
            FCavrnusImportStatus Status;
            Status.StatusMessage = DesiredSceneName;
            Status.SecondaryMessage = FString::Printf(TEXT("Extracting %s"), *FileName);
            Status.Progress = Progress;
            BroadcastStatus(Status);
        });
    };

    // Top-level unzip
    TArray<FString> FirstPass;

    if (!Worker_UnzipAll(TopLevelZip, FileIDFolder, FirstPass, OutError, ReportUnzipProgress))
    {
        OutError = FString::Printf(TEXT("Failed to unzip top-level: %s"), *TopLevelZip);
        return false;
    }
    OutExtractedFiles.Append(FirstPass);

    // Nested zips
    TArray<FString> NestedZips;
    for (const FString& P : FirstPass)
    {
        if (FPaths::GetExtension(P).Equals(TEXT("zip"), ESearchCase::IgnoreCase))
        {
            NestedZips.Add(FPaths::ConvertRelativePathToFull(P));
        }
    }

    for (const FString& ZipPath : NestedZips)
    {
        const FString Base = FPaths::GetBaseFilename(ZipPath); // e.g. "MyAssets"
        const FString TargetFolder = FPaths::Combine(FileIDFolder, Base); // FileIDFolder/MyAssets
        IFileManager::Get().MakeDirectory(*TargetFolder, true);

        TArray<FString> SecondPass;
        FString Err;

        if (!Worker_UnzipAll(ZipPath, TargetFolder, SecondPass, Err, ReportUnzipProgress))
        {
            OutError = FString::Printf(TEXT("Failed to unzip nested: %s (%s)"), *ZipPath, *Err);
            return false;
        }

        // Normalize and append extracted files
        for (const FString& P : SecondPass)
        {
            OutExtractedFiles.Add(FPaths::ConvertRelativePathToFull(P));
        }

        // remove the nested archive after extraction
        FM.Delete(*ZipPath, false, true);
    }


    // 4) Resolve the scene path in FileIDFolder without moving assets
    //    If we accidentally have Cars/Cars.udatasmith, flatten only the scene file one level.
    TArray<FString> SceneCandidates;
    for (const FString& P : OutExtractedFiles)
    {
        if (FPaths::GetExtension(P).Equals(TEXT("udatasmith"), ESearchCase::IgnoreCase))
        {
            SceneCandidates.Add(FPaths::ConvertRelativePathToFull(P));
        }
    }

    if (SceneCandidates.Num() == 0)
    {
        OutError = TEXT("No .udatasmith found after extraction");
        return false;
    }

    // Prefer a scene under FileIDFolder
    FString Chosen = SceneCandidates[0];
    for (const FString& C : SceneCandidates)
    {
        if (FPaths::GetPath(C).Equals(FileIDFolder, ESearchCase::IgnoreCase))
        {
            Chosen = C;
            break;
        }
    }

    // If scene is nested under a same-name folder, move only the scene file up one level
    const FString ParentDir = FPaths::GetPath(Chosen);
    const FString ParentBase = FPaths::GetBaseFilename(ParentDir);
    const FString SceneFileName = FPaths::GetCleanFilename(Chosen);
    const bool bSameNameFolder = ParentBase.Equals(SceneFileName, ESearchCase::IgnoreCase) ||
        ParentBase.Equals(DesiredSceneName, ESearchCase::IgnoreCase);

    if (!ParentDir.Equals(FileIDFolder, ESearchCase::IgnoreCase) && bSameNameFolder)
    {
        const FString DestPath = FPaths::Combine(FileIDFolder, SceneFileName);
        if (!FM.FileExists(*DestPath))
        {
            // Move scene file only; do NOT touch Assets directory
            if (!FM.Move(*DestPath, *Chosen, /*Replace=*/false, /*EvenIfReadOnly=*/true))
            {
                if (FM.Copy(*DestPath, *Chosen, /*Replace=*/false) > 0)
                {
                    FM.Delete(*Chosen, /*RequireExists=*/false, /*EvenIfReadOnly=*/true);
                }
            }
        }
        OutResolvedScenePath = FPaths::ConvertRelativePathToFull(DestPath);
    }
    else
    {
        OutResolvedScenePath = FPaths::ConvertRelativePathToFull(Chosen);
    }

    // Delete the top-level archive once staging is complete
    if (IFileManager::Get().FileExists(*TopLevelZip))
    {
        IFileManager::Get().Delete(*TopLevelZip, /*RequireExists=*/false, /*EvenIfReadOnly=*/true);
        LOG_CAVRNUS_VERBOSE("Deleted top-level archive: %s", *TopLevelZip);
    }

    return true;
}


// The CVT would zip up files into zips and directories into zips and then zip all of those into a zip.
// Newer code just zips the .udatasmith and Assets directory with no internal zips.  This is the cleanup for that mess.
// Moves single-file extraction folders up into ExtractionRoot, removes empty folders and optionally deletes the original archive.
// Returns the resolved path to the requested file if found (empty otherwise).
FString UCavrnusBaseLoader::CleanAndFlattenExtraction(
    const FString& DesiredFileName,
    TArray<FString>& AllExtractedFiles)
{
    // If we have both scene and Assets side-by-side in FileIDFolder, return the scene.
    const FString Root = FPaths::ConvertRelativePathToFull(FileIDFolder);

    FString ScenePath;

    for (const FString& P : AllExtractedFiles)
    {
        const FString Full = FPaths::ConvertRelativePathToFull(P);
        if (FPaths::GetExtension(Full).Equals(TEXT("udatasmith"), ESearchCase::IgnoreCase))
        {
            // Prefer the scene directly under FileIDFolder
            if (FPaths::GetPath(Full).Equals(Root, ESearchCase::IgnoreCase))
            {
                ScenePath = Full;
                break;
            }
            if (ScenePath.IsEmpty())
            {
                ScenePath = Full;
            }
        }
    }

    return ScenePath;
}

void UCavrnusBaseLoader::ResolveFileIdToLocalPathAsync(
    const FString & FileID,
    const FString & FileName,
    const FString & DestinationFolder,
    FResolveFileCallback OnComplete,
    int32 MaxUnzipDepth,
    int32 MaxUnzipIterations)
{
    if (!OnComplete)
    {
        UE_LOG(LogTemp, Error, TEXT("ResolveFileIdToLocalPathAsync: null callback"));
        return;
    }

    // Initialize pump if needed
    if (!PumpInstance)
    {
        PumpInstance = MakeShared<FCavrnusPump>(this);
    }

    // Normalize destination and ensure it exists
    const FString NormalizedDest = NormalizeFullPath(DestinationFolder);
    IFileManager::Get().MakeDirectory(*NormalizedDest, true);

    if (bImportCancelled)
    {
        AsyncTask(ENamedThreads::GameThread, [OnComplete]() {
            OnComplete(false, FString(), TArray<FString>(), TEXT("Resolve cancelled"));
            });
        return;
    }

    // Persist minimal resolve state (depth/iters kept for signature compatibility; staging will not recurse)
    TSharedRef<FResolveState> State = MakeShared<FResolveState>();
    State->FileID = FileID;
    State->FileName = FileName;
    State->Dest = NormalizedDest;
    State->MaxDepth = MaxUnzipDepth;
    State->MaxIters = MaxUnzipIterations;

    FileIDFolder = FPaths::Combine(State->Dest, State->FileID);

    TWeakObjectPtr<UCavrnusBaseLoader> WeakThis(this);

    // Prepare pump request id for the download result
    const int32 DownloadRequestId = PumpInstance->GetNextRequestId();
    LOG_CAVRNUS_VERBOSE("Registering download continuation: ReqId=%d", DownloadRequestId);

    // GameThread continuation for download completion
    PumpInstance->RegisterContinuation(DownloadRequestId,
        [this, WeakThis, State, OnComplete](const FWorkerResultBase& Base)
        {
            const FDownloadResult& R = static_cast<const FDownloadResult&>(Base);

            if (!WeakThis.IsValid())
            {
                OnComplete(false, FString(), TArray<FString>(), TEXT("Loader destroyed"));
                return;
            }

            if (bImportCancelled)
            {
                OnComplete(false, FString(), TArray<FString>(), TEXT("Resolve cancelled"));
                return;
            }

            if (!R.bOk)
            {
                const FString Msg = FString::Printf(TEXT("Download failed for FileID=%s : %s"),
                    *State->FileID, *R.Error);
                UE_LOG(LogTemp, Error, TEXT("%s"), *Msg);
                OnComplete(false, FString(), TArray<FString>(), Msg);
                return;
            }

            // Establish FileIDFolder and normalize path
            const FString LocalFull = NormalizeFullPath(R.LocalPath);
            const FString FileIDFolder = FPaths::Combine(State->Dest, State->FileID);
            IFileManager::Get().MakeDirectory(*FileIDFolder, true);

            // Download succeeded
            FCavrnusImportStatus Status;
            Status.StatusMessage = State->FileName; // e.g. "Cars.udatasmith"
            Status.SecondaryMessage = TEXT("Download complete");
            Status.Progress = 1.0f;
            BroadcastStatus(Status);

            // Non-zip case: move/copy into FileIDFolder and return
            if (!FPaths::GetExtension(LocalFull).Equals(TEXT("zip"), ESearchCase::IgnoreCase))
            {
                const FString TargetPath = FPaths::Combine(FileIDFolder, FPaths::GetCleanFilename(LocalFull));
                bool bMoved = false;

                if (IFileManager::Get().Move(*TargetPath, *LocalFull, /*Replace=*/true, /*EvenIfReadOnly=*/true))
                {
                    bMoved = true;
                }
                else
                {
                    if (IFileManager::Get().Copy(*TargetPath, *LocalFull, /*bReplace=*/true) > 0)
                    {
                        IFileManager::Get().Delete(*LocalFull, /*RequireExists=*/false, /*EvenIfReadOnly=*/true);
                        bMoved = true;
                    }
                }

                const FString ReturnPath = bMoved ? TargetPath : LocalFull;
                TArray<FString> Single; Single.Add(ReturnPath);
                OnComplete(true, ReturnPath, Single, FString());
                return;
            }

            // Zip case: run staging to normalize layout (handles both flat and nested formats)
            {
                TArray<FString> AllExtracted;
                FString ScenePath;
                FString Error;

                // Desired scene name is the requested FileName (e.g., "Cars.udatasmith")
                const FString DesiredSceneName = State->FileName;

                if (!StageArchiveIntoFileIDFolder(LocalFull,
                    DesiredSceneName, AllExtracted, ScenePath, Error))
                {
                    OnComplete(false, ScenePath, AllExtracted, Error.IsEmpty()
                        ? TEXT("Staging failed")
                        : Error);
                    return;
                }

                if (ScenePath.IsEmpty())
                {
                    OnComplete(false, FString(), AllExtracted, TEXT("No .udatasmith found after staging"));
                    return;
                }

                // Success: normalized scene path and extracted list
                OnComplete(true, ScenePath, AllExtracted, FString());
                return;
            }
        });

    // Kick the download worker (no UObject work on threads)
    {
        this->IncrementInflight();
        TWeakPtr<FCavrnusPump> PumpWeak = PumpInstance;
        UCavrnusFunctionLibrary::FetchFileInfoById(FileID,
            CavrnusRemoteContentInfoFunction([this, FileID, NormalizedDest, PumpWeak, DownloadRequestId](const FCavrnusRemoteContent& Info)
            {
                UCavrnusFunctionLibrary::FetchFileByIdToDisk(
                    FileID,
                    NormalizedDest,
                    /* OnProgress */
                    CavrnusContentProgressFunction([this, Info](float Progress, const FString& Step)
                    {
                        auto FormatSize = [](int64 Bytes) -> FString
                            {
                                if (Bytes > 1024 * 1024)
                                    return FString::Printf(TEXT("%.2f MB"), Bytes / 1024.0f / 1024.0f);
                                else if (Bytes > 1024)
                                    return FString::Printf(TEXT("%.1f KB"), Bytes / 1024.0f);
                                else
                                    return FString::Printf(TEXT("%lld bytes"), Bytes);
                            };

                        const int64 TotalSize = Info.Size;
                        const int64 BytesDownloaded = FMath::RoundToInt64(Progress * TotalSize);

                        FCavrnusImportStatus Status;
                        Status.StatusMessage = FString::Printf(TEXT("Downloading %s..."), *Info.FileName);
                        if (TotalSize > 0)
                        {
                            Status.SecondaryMessage = FString::Printf(TEXT("%s / %s"), *FormatSize(BytesDownloaded), *FormatSize(TotalSize));
                        }
                        else
                        {
                            Status.SecondaryMessage = Step;
                        }
                        Status.Progress = Progress;
                        BroadcastStatus(Status);
                    }),
                    /* OnContentLoaded */
                    TFunction<void(FString)>([this, DownloadRequestId, PumpWeak, FileID](const FString& LocalPath)
                    {
                        FDownloadResult Res;
                        Res.RequestId = DownloadRequestId;
                        Res.FileID = FileID;
                        Res.bOk = true;
                        Res.LocalPath = LocalPath;

                        LOG_CAVRNUS_VERBOSE("Download complete: ReqId=%d, FileID=%s, Path=%s",
                            DownloadRequestId, *FileID, *LocalPath);

                        if (TSharedPtr<FCavrnusPump> PumpCopy = PumpWeak.Pin())
                        {
                            TUniquePtr<FWorkerResultBase> Ptr = MakeUnique<FDownloadResult>(MoveTemp(Res));
                            PumpCopy->PushResult(MoveTemp(Ptr));
                        }
                        this->DecrementInflight();
                    }),
                    /* OnFailure */
                    CavrnusError([this, DownloadRequestId, PumpWeak, FileID](const FString& Error)
                    {
                        FDownloadResult Res;
                        Res.RequestId = DownloadRequestId;
                        Res.FileID = FileID;
                        Res.bOk = false;
                        Res.Error = Error;

                        LOG_CAVRNUS_VERBOSE("Download failed: ReqId=%d, FileID=%s, Error=%s",
                            DownloadRequestId, *FileID, *Error);

                        if (TSharedPtr<FCavrnusPump> PumpCopy = PumpWeak.Pin())
                        {
                            TUniquePtr<FWorkerResultBase> Ptr = MakeUnique<FDownloadResult>(MoveTemp(Res));
                            PumpCopy->PushResult(MoveTemp(Ptr));
                        }
                        this->DecrementInflight();
                    }));
            }));
    }
}



// ProcessExtractionQueue - rewritten as GameThread-driven loop with worker dispatch
// This function is intended to run on GameThread. It will dispatch unzip workers for heavy work and return;
// continuation is driven by pump results which will re-call this function on GameThread.

void UCavrnusBaseLoader::ProcessExtractionQueue(
    const TSharedRef<FResolveState>& State,
    const FString& LocalFullCopy,
    const TSharedRef<TArray<FString>, ESPMode::ThreadSafe>& Queue,
    const TSharedRef<TArray<FString>, ESPMode::ThreadSafe>& AllExtracted,
    const TSharedRef<TSet<FString>, ESPMode::ThreadSafe>& Processed,
    const TSharedRef<int32, ESPMode::ThreadSafe>& ReadIndex,
    const TSharedRef<int32, ESPMode::ThreadSafe>& Iterations,
    TFunction<void(bool, const FString&, const TArray<FString>&, const FString&)> LocalOnComplete)
{
    check(IsInGameThread());

    if (bImportCancelled)
    {
        LocalOnComplete(false, FString(), *AllExtracted, TEXT("Resolve cancelled"));
        return;
    }

    if (*Iterations >= State->MaxIters)
    {
        LocalOnComplete(false, FString(), *AllExtracted, TEXT("Exceeded max iterations"));
        return;
    }

    while (true)
    {
        int32 Index = (*ReadIndex)++;
        if (Index >= Queue->Num())
        {
            // Final resolution attempt using the new CleanAndFlattenExtraction
            TArray<FString> Dump = *AllExtracted;

            // ExtractionRoot is the FileIDFolder in your flow
            const FString Resolved = CleanAndFlattenExtraction(
                State->FileName,       // Desired .udatasmith filename
                Dump);                 // All extracted files

            if (!Resolved.IsEmpty())
            {
                LocalOnComplete(true, Resolved, Dump, FString());
                return;
            }

            // Fallback: search for requested file name among extracted files
            for (const FString& Candidate : Dump)
            {
                if (FPaths::GetCleanFilename(Candidate).Equals(State->FileName, ESearchCase::IgnoreCase))
                {
                    LocalOnComplete(true, Candidate, Dump, FString());
                    return;
                }
            }

            LocalOnComplete(false, FString(), Dump,
                FString::Printf(TEXT("Requested file '%s' not found"), *State->FileName));
            return;
        }

        const FString ZipPath = NormalizeFullPath((*Queue)[Index]);
        if (Processed->Contains(ZipPath))
        {
            continue; // already processed this zip
        }

        Processed->Add(ZipPath);
        (*Iterations)++;

        const FString BaseName = FPaths::GetBaseFilename(ZipPath);
        const FString DestForZip = FPaths::Combine(FileIDFolder, BaseName);
        IFileManager::Get().MakeDirectory(*DestForZip, true);

        if (!PumpInstance)
        {
            PumpInstance = MakeShared<FCavrnusPump>(this);
        }

        const int32 ReqId = PumpInstance->GetNextRequestId();

        // Register continuation: runs on GameThread when worker finishes
        PumpInstance->RegisterContinuation(ReqId,
            [this, State, LocalFullCopy,
            Queue, AllExtracted, Processed, ReadIndex, Iterations,
            LocalOnComplete, ZipPath](const FWorkerResultBase& Base)
            {
                const FUnzipResult& R = static_cast<const FUnzipResult&>(Base);

                if (!R.bOk)
                {
                    FCavrnusImportStatus Warn;
                    Warn.StatusMessage = TEXT("Partial unzip failure");
                    Warn.SecondaryMessage = FString::Printf(TEXT("Skipped: %s (%s)"), *R.ZipPath, *R.Error);
                    Warn.Progress = 1.0f;
                    Warn.bSuccess = true;
                    BroadcastStatus(Warn);

                    UE_LOG(LogTemp, Warning, TEXT("Unzip failed: %s (%s)"), *R.ZipPath, *R.Error);
                    ProcessExtractionQueue(State, LocalFullCopy,
                        Queue, AllExtracted, Processed, ReadIndex, Iterations, LocalOnComplete);
                    return;
                }

                FCavrnusImportStatus Final;
                Final.StatusMessage = State->FileName;
                Final.SecondaryMessage = FString::Printf(TEXT("Unzip Complete: %d files"), R.ExtractedFiles.Num());
                Final.Progress = 1.0f;
                Final.bSuccess = true;
                BroadcastStatus(Final);

                // merge extracted files...
                for (const FString& Path : R.ExtractedFiles)
                {
                    const FString Norm = NormalizeFullPath(Path);
                    if (!AllExtracted->Contains(Norm))
                        AllExtracted->Add(Norm);
                    if (FPaths::GetExtension(Norm).Equals(TEXT("zip"), ESearchCase::IgnoreCase))
                        if (!Processed->Contains(Norm))
                            Queue->Add(Norm);
                }

                ProcessExtractionQueue(State, LocalFullCopy,
                    Queue, AllExtracted, Processed, ReadIndex, Iterations, LocalOnComplete);
            });

        this->IncrementInflight();

        Async(EAsyncExecution::ThreadPool, [this, ZipPath, DestForZip, ReqId]()
            {
                FUnzipResult R;
                R.RequestId = ReqId;
                R.ZipPath = ZipPath;
                R.DestFolder = DestForZip; // use the per-zip destination we created on GameThread

                bool bOk = Worker_UnzipAll(ZipPath, DestForZip, R.ExtractedFiles, R.Error,
                    [this](const FString& FileName, float Progress)
                    {
                        FCavrnusImportStatus Status;
                        Status.StatusMessage = TEXT("Extracting Archive");
                        Status.SecondaryMessage = FileName;
                        Status.Progress = Progress;
                        BroadcastStatus(Status);
                    });

                R.bOk = bOk;

                if (PumpInstance)
                {
                    TUniquePtr<FWorkerResultBase> Ptr = MakeUnique<FUnzipResult>(MoveTemp(R));
                    PumpInstance->PushResult(MoveTemp(Ptr));
                }

                this->DecrementInflight();
            });

        return; // yield until the worker continuation resumes
    }
}
