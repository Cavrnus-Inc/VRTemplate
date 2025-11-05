#include "FileImporter/DatasmithFileImporter.h"
#include "DatasmithRuntime.h"
#include "TimerManager.h"
#include "Engine/World.h"

bool ADatasmithFileImporter::CanImport(const FString& FileExtension) const
{
    return FileExtension.Equals(TEXT("udatasmith"), ESearchCase::IgnoreCase);
}

void ADatasmithFileImporter::ImportFileInternal(const FString& FilePath, const FCavrnusImportSettings& Settings)
{
    SourcePath = FilePath;
    bWaitingForLoadStart = true;

    // Prevent GC while import is in progress
    AddToRoot();

    UWorld* World = GetWorld();
    if (!World)
    {
        FCavrnusImportStatus FailStatus;
        FailStatus.StatusMessage = TEXT("Invalid world context");
        FailStatus.bSuccess = false;
        OnImportComplete.Broadcast(FailStatus);
        return;
    }

    ADatasmithRuntimeActor* Actor = World->SpawnActor<ADatasmithRuntimeActor>();
    if (!Actor)
    {
        FCavrnusImportStatus FailStatus;
        FailStatus.StatusMessage = TEXT("Failed to spawn Datasmith runtime actor");
        FailStatus.bSuccess = false;
        OnImportComplete.Broadcast(FailStatus);
        return;
    }

    TargetActor = Actor;

    // Initial status
    {
        FCavrnusImportStatus Status;
        Status.Progress = 0.0f;
        Status.StatusMessage = TEXT("Starting Datasmith import...");
        Status.bSuccess = true;
        OnImportStatusUpdate.Broadcast(Status);
    }

    Actor->LoadFile(SourcePath);

    // Poll actor status until complete
    World->GetTimerManager().SetTimer(PollTimerHandle, this, &ADatasmithFileImporter::PollActorStatus, 0.1f, true);
}

void ADatasmithFileImporter::PollActorStatus()
{
    if (!TargetActor.IsValid())
    {
        FCavrnusImportStatus Status;
        Status.Progress = 1.0f;
        Status.StatusMessage = TEXT("Import failed: Actor destroyed");
        Status.bSuccess = false;
        OnImportComplete.Broadcast(Status);

        GetWorld()->GetTimerManager().ClearTimer(PollTimerHandle);
        RemoveFromRoot();
        MarkAsGarbage();
        return;
    }

    ADatasmithRuntimeActor* Actor = TargetActor.Get();
    const bool bIsBuilding = Actor->bBuilding;
    const bool bIsReceiving = Actor->IsReceiving();

    if (bWaitingForLoadStart)
    {
        if (bIsBuilding || bIsReceiving)
        {
            bWaitingForLoadStart = false;
            FCavrnusImportStatus Status;
            Status.Progress = Actor->Progress;
            Status.StatusMessage = TEXT("Datasmith import started...");
            Status.bSuccess = true;
            OnImportStatusUpdate.Broadcast(Status);
        }
        return;
    }

    if (bIsBuilding || bIsReceiving)
    {
        FCavrnusImportStatus Status;
        Status.Progress = Actor->Progress; // Arbitrary mid-progress
        Status.StatusMessage = TEXT("Datasmith import in progress...");
        Status.bSuccess = true;
        OnImportStatusUpdate.Broadcast(Status);
        return;
    }

    // Finished
    FCavrnusImportStatus FinalStatus;
    FinalStatus.Progress = Actor->Progress;
    FinalStatus.StatusMessage = TEXT("Datasmith import complete");
    FinalStatus.bSuccess = true;
    FinalStatus.ImportedAssets.Add(Actor);

    OnImportComplete.Broadcast(FinalStatus);

    GetWorld()->GetTimerManager().ClearTimer(PollTimerHandle);
    RemoveFromRoot();
    MarkAsGarbage();
}
