#include "FileImporter/DatasmithFileImporter.h"
#include "Utilities/ActorOptimizationLibrary.h"
#include "DatasmithRuntime.h"
#include "TimerManager.h"
#include "FileImporter/DatasmithUtilities.h"
#include "FileImporter/DatasmithRepairProcessor.h"
#include "CavrnusConnectorSettings.h"
#include "Engine/World.h"

bool UDatasmithFileImporter::CanImport(const FString& FileExtension) const
{
    return FileExtension.Equals(TEXT("udatasmith"), ESearchCase::IgnoreCase);
}

void UDatasmithFileImporter::ImportFileInternal(const FString& FilePath, const FCavrnusImportSettings& Settings)
{
    bWaitingForLoadStart = true;

    UWorld* World = GetWorld();
    if (!World)
    {
        FCavrnusImportStatus FailStatus;
        FailStatus.StatusMessage = TEXT("Invalid world context");
        FailStatus.bSuccess = false;
        FailStatus.SecondaryMessage = FPaths::GetCleanFilename(TrackedFilePath);
        OnImportComplete.Broadcast(FailStatus);
        return;
    }

    ADatasmithRuntimeActor* Actor = World->SpawnActor<ADatasmithRuntimeActor>();
    if (!Actor)
    {
        FCavrnusImportStatus FailStatus;
        FailStatus.StatusMessage = TEXT("Failed to spawn Datasmith runtime actor");
        FailStatus.bSuccess = false;
        FailStatus.SecondaryMessage = FPaths::GetCleanFilename(TrackedFilePath);
        OnImportComplete.Broadcast(FailStatus);
        return;
    }

    TargetActor = Actor;

    // Initial status
    {
        FCavrnusImportStatus Status;
        Status.Progress = 0.0f;
        Status.StatusMessage = TEXT("Starting Datasmith import...");
        Status.SecondaryMessage = FPaths::GetCleanFilename(TrackedFilePath);
        Status.bSuccess = true;
        OnImportStatusUpdate.Broadcast(Status);
    }

    Actor->LoadFile(FilePath);

    // Poll actor status until complete
    World->GetTimerManager().SetTimer(PollTimerHandle, this, &UDatasmithFileImporter::PollActorStatus, 0.1f, true);
}

void UDatasmithFileImporter::PollActorStatus()
{
    if (!TargetActor.IsValid())
    {
        FCavrnusImportStatus Status;
        Status.Progress = 1.0f;
        Status.StatusMessage = TEXT("Import failed: Actor destroyed");
        Status.bSuccess = false;
        Status.SecondaryMessage = FPaths::GetCleanFilename(TrackedFilePath);
        BroadcastComplete(Status);

        GetWorld()->GetTimerManager().ClearTimer(PollTimerHandle);
        TargetActor = nullptr;
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
            Status.StatusMessage = TEXT("Datasmith import starting...");
            Status.SecondaryMessage = FPaths::GetCleanFilename(TrackedFilePath);;
            Status.bSuccess = true;
            BroadcastStatus(Status);
        }
        return;
    }

    if (bIsBuilding || bIsReceiving)
    {
        FCavrnusImportStatus Status;
        Status.Progress = Actor->Progress; // Arbitrary mid-progress
        Status.StatusMessage = TEXT("Datasmith import in progress...");
        Status.SecondaryMessage = FPaths::GetCleanFilename(TrackedFilePath);;
        Status.bSuccess = true;
        BroadcastStatus(Status);
        return;
    }

    // Finished
    FCavrnusImportStatus FinalStatus;
    FinalStatus.Progress = Actor->Progress;
    FinalStatus.StatusMessage = FPaths::GetCleanFilename(TrackedFilePath);
    FinalStatus.SecondaryMessage = TEXT("Datasmith import complete");
    FinalStatus.bSuccess = true;
    FinalStatus.ImportedAssets.Add(Actor);

    BroadcastComplete(FinalStatus);

    // If Twinmotion material files

    UDatasmithFileImporter::ProcessTwinmotionDatasmithMaterials(TrackedFilePath, Actor);

    GetWorld()->GetTimerManager().ClearTimer(PollTimerHandle);
    TargetActor = nullptr;
}

bool UDatasmithFileImporter::AdditionalValidation(const FString& NormalizedFilePath, FString& StatusMessage)
{
	if (UCavrnusConnectorSettings::Get()->bIgnoreVersionCheck)
	{
		return true;
	}
    return UDatasmithFileLibrary::IsCompatibleWithEngineVersion(NormalizedFilePath, StatusMessage);
}

void UDatasmithFileImporter::CancelImport()
{
    if (TargetActor.IsValid())
    {
        TargetActor->Destroy();
        TargetActor = nullptr;
    }

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(PollTimerHandle);
    }

    PollTimerHandle.Invalidate();
    TargetActor = nullptr;
}

void UDatasmithFileImporter::BeginDestroy()
{
    if (bImportCancelled) // Clean ourselves up if the File Importer is destroyed before the Actor completes loading
        CancelImport();
    Super::BeginDestroy();
}

int UDatasmithFileImporter::ProcessTwinmotionDatasmithChildUsingSlotNames(const AActor* Actor)
{
    if (!Actor)
        return 0;
    int FixCount = 0;

    TArray<UStaticMeshComponent*> MeshComponents;
    Actor->GetComponents<UStaticMeshComponent>(MeshComponents, true);

    for (UStaticMeshComponent* MeshComp : MeshComponents)
    {
        if (!MeshComp)
            continue;
        MeshComp->SetForcedLodModel(1);

        TArray<FName> SlotNames = MeshComp->GetMaterialSlotNames();
        int32 NumMaterials = SlotNames.Num();

        bool bModified = false;

        for (int32 i = 0; i < NumMaterials; ++i)
        {
            UMaterialInterface* Material = MeshComp->GetMaterial(i);

            if (!Material)
            {
                for (int j = i + 1; j < NumMaterials; ++j)
                {
                    if (SlotNames[i] == SlotNames[j])
                    {
                        MeshComp->SetMaterial(i, MeshComp->GetMaterial(j));
                        bModified = true;
                        FixCount++;
                        break;
                    }
                }
            }
        }

        if (bModified)
        {
            MeshComp->MarkRenderStateDirty();
        }
    }
    return FixCount;
}

void UDatasmithFileImporter::ProcessTwinmotionDatasmithMaterials(const FString& FilePath, ADatasmithRuntimeActor* DatasmithActor)
{
    if (!DatasmithActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("DatasmithActor is null."));
        return;
    }
    if (FilePath.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("FilePath for Datasmith Actor is Empty.  Unable to determine type and further process"));
    }
    TArray<AActor*> ActorsToProcess;

    GetAllRelevantActorsRecursive(DatasmithActor, ActorsToProcess);

    UE_LOG(LogTemp, Error, TEXT("XXXXX - Found %d from GetAllStaticMeshActorsRecursive"), ActorsToProcess.Num());

    int TotalFixed = 0;

    int count = 0;

    FString ErrorMessage;
    EDatasmithRuntimeFileType FileType = UDatasmithFileLibrary::GetFileTypeFromDatasmithFile(FilePath, ErrorMessage);

    switch (FileType)
    {
        case EDatasmithRuntimeFileType::Twinmotion:
        {
            // Process Twinmotion specific properties
            TotalFixed += ProcessTwinmotionDatasmithChildUsingSlotNames(DatasmithActor);
            for (AActor* Actor : ActorsToProcess)
            {
                TotalFixed += ProcessTwinmotionDatasmithChildUsingSlotNames(Actor);
            }
            break;
        }
        default:
        {
            // Nothing to do here yet.
        }
    }
}
void UDatasmithFileImporter::GetAllRelevantActorsRecursive(AActor* RootActor, TArray<AActor*>& OutActors)
{
    // Delegate to the shared implementation in UActorOptimizationLibrary
    UActorOptimizationLibrary::GetAllActorsRecursive(RootActor, OutActors);
}

