#include "FileImporter/CavrnusFileImporter.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

void UCavrnusFileImporter::ImportFile(
    const FString& FilePath,
    const FCavrnusImportSettings& Settings,
    FOnImportStatusUpdateDynamic OnStatusUpdate,
    FOnImportCompleteDynamic OnComplete)
{
    TrackedFilePath = FilePath;
    // Bind delegates if provided
    if (OnStatusUpdate.IsBound())
    {
        FScriptDelegate ScriptDelegate;
        ScriptDelegate.BindUFunction(OnStatusUpdate.GetUObject(), OnStatusUpdate.GetFunctionName());
        OnImportStatusUpdate.Add(ScriptDelegate);
    }

    if (OnComplete.IsBound())
    {
        FScriptDelegate ScriptDelegate;
        ScriptDelegate.BindUFunction(OnComplete.GetUObject(), OnComplete.GetFunctionName());
        OnImportComplete.Add(ScriptDelegate);
    }

    // Validate file early and fail fast if missing or invalid
    if (!ValidateSourceFileOrBroadcastFailure(FilePath))
    {
        // ValidateSourceFileOrBroadcastFailure already called BroadcastImportFailure
        return;
    }

    // Proceed with actual import
    ImportFileInternal(FilePath, Settings);
}

void UCavrnusFileImporter::CancelImport()
{
    bImportCancelled = true;
}

void UCavrnusFileImporter::BeginDestroy()
{
    CancelImport(); // your subclass should override this to cancel timers, latent actions, etc.
    Super::BeginDestroy();
}

bool UCavrnusFileImporter::ValidateSourceFileOrBroadcastFailure(const FString& InFilePath)
{
    // Normalize and collapse to reduce false negatives
    FString FilePath = InFilePath;
    FPaths::NormalizeFilename(FilePath);
    FPaths::CollapseRelativeDirectories(FilePath);

    // Empty path check
    if (FilePath.IsEmpty())
    {
        BroadcastImportFailure(TEXT("Import failed: empty file path provided."));
        return false;
    }

    // Existence check
    if (!IFileManager::Get().FileExists(*FilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("ValidateSourceFileOrBroadcastFailure: file not found: %s"), *FilePath);
        BroadcastImportFailure(FString::Printf(TEXT("Import failed: file not found '%s'"), *FilePath));
        return false;
    }

    // Loader-specific checks
    FString Message;
    if (!AdditionalValidation(FilePath, Message))
    {
        BroadcastImportFailure(Message);
        // AdditionalValidation should log specifics when it fails.
        return false;
    }
    else if (Message != "")
    {
		FCavrnusImportStatus VersionCheck;
		VersionCheck.bSuccess = true;
		VersionCheck.Progress = 0.0f;
		VersionCheck.StatusMessage = "Validation notice:";
		VersionCheck.SecondaryMessage = Message;
        BroadcastStatus(VersionCheck);
    }
    return true;
}

bool UCavrnusFileImporter::AdditionalValidation(const FString& /*NormalizedFilePath*/, FString& StatusMessage)
{
    StatusMessage = "";
    // Default: no extra checks
    return true;
}

void UCavrnusFileImporter::BroadcastImportFailure(const FString& Message)
{
    // Build the import status (use your project's struct FCavrnusImportStatus)
    FCavrnusImportStatus Status;
    Status.bSuccess = false;
    Status.Progress = 0.0f;
    Status.StatusMessage = Message;
    Status.SecondaryMessage = TrackedFilePath;
    UE_LOG(LogTemp, Error, TEXT("%s"), *Message);

    // Broadcast via existing multi-cast so Blueprints/UI are notified
    BroadcastComplete(Status);
}
