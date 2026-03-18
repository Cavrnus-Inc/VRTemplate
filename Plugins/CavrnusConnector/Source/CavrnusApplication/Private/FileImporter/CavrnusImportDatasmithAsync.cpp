#include "FileImporter/CavrnusImportDatasmithAsync.h"

UCavrnusImportDatasmithAsync* UCavrnusImportDatasmithAsync::ImportDatasmithFile(
    UObject* WorldContextObject,
    const FString& FilePath,
    const FCavrnusImportSettings& Settings)
{
    UCavrnusImportDatasmithAsync* Node = NewObject<UCavrnusImportDatasmithAsync>();
    Node->WorldContextObject = WorldContextObject;
    Node->FilePath = FilePath;
    Node->Settings = Settings;
    Node->RegisterWithGameInstance(WorldContextObject);

    return Node;
}

void UCavrnusImportDatasmithAsync::Activate()
{
    if (!WorldContextObject)
    {
        FCavrnusImportStatus ErrorStatus;
        ErrorStatus.bSuccess = false;
        ErrorStatus.StatusMessage = TEXT("Import failed: Missing world context");
        OnComplete.Broadcast(ErrorStatus);
        SetReadyToDestroy();
        return;
    }

    Importer = NewObject<UDatasmithFileImporter>(WorldContextObject);
    if (!Importer)
    {
        FCavrnusImportStatus ErrorStatus;
        ErrorStatus.bSuccess = false;
        ErrorStatus.StatusMessage = TEXT("Failed to create importer");
        OnComplete.Broadcast(ErrorStatus);
        SetReadyToDestroy();
        return;
    }

    Importer->OnImportStatusUpdate.AddDynamic(this, &UCavrnusImportDatasmithAsync::HandleStatus);
    Importer->OnImportComplete.AddDynamic(this, &UCavrnusImportDatasmithAsync::HandleComplete);

    Importer->ImportFile(FilePath, Settings, FOnImportStatusUpdateDynamic(), FOnImportCompleteDynamic());
}

void UCavrnusImportDatasmithAsync::HandleStatus(const FCavrnusImportStatus& Status)
{
    OnStatusUpdate.Broadcast(Status);
}

void UCavrnusImportDatasmithAsync::HandleComplete(const FCavrnusImportStatus& FinalStatus)
{
    OnComplete.Broadcast(FinalStatus);
    SetReadyToDestroy(); // allow GC after completion
}
