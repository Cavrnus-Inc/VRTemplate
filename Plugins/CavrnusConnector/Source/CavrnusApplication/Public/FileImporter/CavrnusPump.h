#pragma once
#include "CoreMinimal.h"
#include "FileImporter/WorkerResults.h"
#include "Containers/Queue.h"
#include <atomic>

class UCavrnusBaseLoader;   // forward declare only

class FCavrnusPump
{
public:
    explicit FCavrnusPump(UCavrnusBaseLoader* InOwner);

    int32 GetNextRequestId();
    void PushResult(TUniquePtr<FWorkerResultBase>&& Result);
    void RegisterContinuation(int32 RequestId, TFunction<void(const FWorkerResultBase&)> Continuation);
    void Drain();
    void ScheduleDrain();

private:
    UCavrnusBaseLoader* Owner = nullptr;
    TQueue<TUniquePtr<FWorkerResultBase>, EQueueMode::Mpsc> Queue;
    TMap<int32, TFunction<void(const FWorkerResultBase&)>> Continuations;
    std::atomic<bool> bDrainScheduled;
    int32 NextRequestId;
};