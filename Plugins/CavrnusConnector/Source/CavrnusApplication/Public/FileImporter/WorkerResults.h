#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
/**
 * Base type for all worker results.
 * Plain POD, safe to move across threads.
 */
struct FWorkerResultBase
{
    int32 RequestId = -1;
    virtual ~FWorkerResultBase() = default;
};

/**
 * Result of a download worker.
 */
struct FDownloadResult : public FWorkerResultBase
{
    bool bOk = false;
    FString LocalPath;
    FString Error;
    FString FileID;
};

/**
 * Result of an unzip worker.
 */
struct FUnzipResult : public FWorkerResultBase
{
    bool bOk = false;
    FString ZipPath;
    FString DestFolder;
    TArray<FString> ExtractedFiles;
    FString Error;
};


class FCompletionQueue
{
public:
    void Enqueue(TUniquePtr<FWorkerResultBase>&& Item)
    {
        Queue.Enqueue(MoveTemp(Item));
    }

    bool Dequeue(TUniquePtr<FWorkerResultBase>& Out)
    {
        return Queue.Dequeue(Out);
    }

    bool IsEmpty() const
    {
        return Queue.IsEmpty();
    }

private:
    TQueue<TUniquePtr<FWorkerResultBase>, EQueueMode::Mpsc> Queue;
};

