#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CavrnusFunctionLibrary.h"
#include "Core/DisposableUObject.h"
#include "Managers/SpawnedObjects/CavrnusImportDelegates.h"
#include "Abstract/FileImporter/CavrnusBaseLoader_Abstract.h"
#include "Delegates/Delegate.h"
#include "Engine/World.h"
#include "FileImporter/CavrnusPump.h"
#include "CavrnusBaseLoader.generated.h"

typedef TFunction<void(float, const FString&)> CavrnusContentProgressCallback;
typedef TFunction<void(const FString&)>        CavrnusErrorCallback;
typedef TFunction<void(const FString&, const FString&)> CavrnusFileContentCallback;
// Native C++
DECLARE_DELEGATE_TwoParams(FCavrnusContentProgressNative, float, const FString&);
DECLARE_DELEGATE_OneParam(FCavrnusContentFileNative, const FString&);
DECLARE_DELEGATE_OneParam(FCavrnusContentErrorNative, const FString&);

// Dynamic (Blueprint single-cast)
DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusContentErrorDynamic, FString, Error);

// Dynamic multicast (BlueprintAssignable)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCavrnusContentProgressEvent, float, Progress, FString, Step);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCavrnusContentFileEvent, FString, FileDest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCavrnusContentErrorEvent, FString, Error);

// Per-resolve shared state used by ResolveFileIdToLocalPathAsync and ProcessExtractionQueue
struct FResolveState
{
    FString FileID;
    FString FileName;
    FString Dest;
    int32 MaxDepth = 0;
    int32 MaxIters = 0;
};

// Toggle verbose thread logs for debugging
#define CAVRNUS_VERBOSE_THREAD_LOGS 0

#if CAVRNUS_VERBOSE_THREAD_LOGS
#define LOG_CAVRNUS_VERBOSE(Format, ...) UE_LOG(LogTemp, Verbose, TEXT("[CavrnusLoader] ") Format, ##__VA_ARGS__)
#else
#define LOG_CAVRNUS_VERBOSE(Format, ...)
#endif

/**
 * Abstract base class for all Cavrnus loaders.
 * Loaders are transient UObjects: they process a load and then self-destruct.
 */
UCLASS(Abstract, Blueprintable)
class CAVRNUSAPPLICATION_API UCavrnusBaseLoader : public UCavrnusBaseLoader_Abstract
{
    GENERATED_BODY()

public:
    virtual void BeginDestroy() override;

    // --- Blueprint dynamic delegates (BlueprintAssignable) ---
    UPROPERTY(BlueprintAssignable, Category = "Cavrnus|Loader")
    FOnCavrnusImportStatusUpdate OnStatusUpdateBP;

    UPROPERTY(BlueprintAssignable, Category = "Cavrnus|Loader")
    FOnCavrnusImportComplete OnCompleteBP;

    /** Returns true if this loader can handle the given wellKnownObjectId */
    UFUNCTION(BlueprintNativeEvent, Category = "Cavrnus|Loader")
    bool CanHandleId(const FString& WellKnownObjectId) const;

    /** Entry point: start the load */
    UFUNCTION(BlueprintCallable, Category = "Cavrnus|Loader")
    void StartLoad(const FCavrnusSpawnedObject& ObjectData, UWorld* World);

    /** Cancel the load (optional override) */
    UFUNCTION(BlueprintCallable, Category = "Cavrnus|Loader")
    virtual void CancelLoad();

    /** Post process function to finalize the load. Guaranteed to run after loading has completed */
    UFUNCTION(BlueprintCallable, Category = "Cavrnus|Loader")
    virtual void FinalizeLoad();

    // --- Content download events: Blueprint + Native ---
    UPROPERTY(BlueprintAssignable, Category = "Cavrnus|Loader")
    FOnCavrnusContentProgressEvent OnProgressBP;

    UPROPERTY(BlueprintAssignable, Category = "Cavrnus|Loader")
    FOnCavrnusContentFileEvent OnContentLoadedBP;

    UPROPERTY(BlueprintAssignable, Category = "Cavrnus|Loader")
    FOnCavrnusContentErrorEvent OnFailureBP;

    // Native
    FCavrnusContentProgressNative OnProgressNative;
    FCavrnusContentFileNative OnContentLoadedNative;
    FCavrnusContentErrorNative OnFailureNative;

    TSharedPtr<FCavrnusPump> GetPumpInstance() const { return PumpInstance; }

    // --- Broadcast helpers ---
    void ReportContentLoaded(const FString& Path);
    void ReportFailure(const FString& Error);

protected:
    UPROPERTY()
    FString FileIDFolder; // Used for toast message tracking

    /** Subclasses implement this to perform the actual load */
    virtual void DoLoadInternal(const FCavrnusSpawnedObject& ObjectData, UWorld* World) PURE_VIRTUAL(UCavrnusBaseLoader::DoLoadInternal, );

    /** Helper to broadcast status to both Blueprint and C++ */
    void BroadcastStatus(const FCavrnusImportStatus& Status);

    /** Helper to broadcast completion to both Blueprint and C++ */
    void BroadcastComplete(const FCavrnusImportStatus& FinalStatus);

    /** Utility: parse a JSON string containing FileID/FileName into out params.
     *  Returns true if parsing succeeded. */
    bool ParseFileInfoJson(const FString& JsonString, FString& OutFileID, FString& OutFileName) const;

    // Resolve helper: download + unzip (recursive) and return a resolved path plus all extracted files.
    // OnComplete signature: TFunction<void(bool bSuccess, const FString& ResolvedPath, const TArray<FString>& AllExtractedFiles, const FString& ErrorMessage)>
    using FResolveFileCallback = TFunction<void(bool, const FString&, const TArray<FString>&, const FString&)>;

void ProcessExtractionQueue(
    const TSharedRef<FResolveState>& State,
    const FString& LocalFullCopy,
    const TSharedRef<TArray<FString>, ESPMode::ThreadSafe>& Queue,
    const TSharedRef<TArray<FString>, ESPMode::ThreadSafe>& AllExtracted,
    const TSharedRef<TSet<FString>, ESPMode::ThreadSafe>& Processed,
    const TSharedRef<int32, ESPMode::ThreadSafe>& ReadIndex,
    const TSharedRef<int32, ESPMode::ThreadSafe>& Iterations,
    TFunction<void(bool, const FString&, const TArray<FString>&, const FString&)> LocalOnComplete);

    void ResolveFileIdToLocalPathAsync(
        const FString& FileID,
        const FString& FileName,
        const FString& DestinationFolder,
        FResolveFileCallback OnComplete,
        int32 MaxUnzipDepth = 5,
        int32 MaxUnzipIterations = 1000);

    bool StageArchiveIntoFileIDFolder(
        const FString& TopLevelZip,
        const FString& DesiredSceneName, // "Cars.udatasmith"
        TArray<FString>& OutExtractedFiles,
        FString& OutResolvedScenePath,
        FString& OutError);

    FString CleanAndFlattenExtraction(
        const FString& DesiredFileName,
        TArray<FString>& AllExtractedFiles);

    /** Utility to normalize full path (helper used by implementation) */
    static FString NormalizeFullPath(const FString& In);

    /** Track cancellation state */
    bool bImportCancelled = false;

    TWeakObjectPtr<UWorld> WorldContext;
private:
    // pump instance used by the refactored async pipeline; owned by this loader
    TSharedPtr<FCavrnusPump> PumpInstance;

    // Tracks inflight ops for this loader instance only
    std::atomic<int32> InflightOps{ 0 };

    bool bRootedForInflight = false;

    void IncrementInflight();
    void DecrementInflight();

    void FinishDestroy();
};
