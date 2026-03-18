#include "FileImporter/CavrnusPump.h"
#include "FileImporter/CavrnusBaseLoader.h"
#include "Async/Async.h"

FCavrnusPump::FCavrnusPump(UCavrnusBaseLoader* InOwner)
    : Owner(InOwner), bDrainScheduled(false), NextRequestId(1)
{}

// … implement GetNextRequestId, PushResult, RegisterContinuation, Drain …

void FCavrnusPump::ScheduleDrain()
{
    bool expected = false;
    if (bDrainScheduled.compare_exchange_strong(expected, true))
    {
        TWeakObjectPtr<UCavrnusBaseLoader> WeakOwner(Owner);
        AsyncTask(ENamedThreads::GameThread, [WeakOwner]()
        {
            if (UCavrnusBaseLoader* Self = WeakOwner.Get())
            {
                if (TSharedPtr<FCavrnusPump> Pump = Self->GetPumpInstance())
                {
                    Pump->Drain();
                }
            }
        });
    }
}

int32 FCavrnusPump::GetNextRequestId()
{
    return NextRequestId++;
}

void FCavrnusPump::PushResult(TUniquePtr<FWorkerResultBase>&& Result)
{
    Queue.Enqueue(MoveTemp(Result));
    ScheduleDrain();
}

void FCavrnusPump::RegisterContinuation(int32 RequestId, TFunction<void(const FWorkerResultBase&)> Continuation)
{
    check(IsInGameThread());
    Continuations.Add(RequestId, MoveTemp(Continuation));
}

void FCavrnusPump::Drain()
{
    check(IsInGameThread());
    bDrainScheduled.store(false);
    TUniquePtr<FWorkerResultBase> Item;
    while (Queue.Dequeue(Item))
    {
        if (!Item) continue;
        int32 Req = Item->RequestId;
        if (TFunction<void(const FWorkerResultBase&)>* Fn = Continuations.Find(Req))
        {
            (*Fn)(*Item);
            Continuations.Remove(Req);
        }
    }
}