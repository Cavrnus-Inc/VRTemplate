#pragma once

#include "Managers/SpawnedObjects/CavrnusImportDelegates.h"
#include "CavrnusFileImporter.generated.h"

UCLASS(Abstract, Blueprintable)
class CAVRNUSAPPLICATION_API UCavrnusFileImporter : public UObject
{
    GENERATED_BODY()

public:

	virtual void BeginDestroy() override;
    // Persistent multicast events
    UPROPERTY(BlueprintAssignable, Category = "Cavrnus|Import")
    FOnCavrnusImportStatusUpdate OnImportStatusUpdate;

    UPROPERTY(BlueprintAssignable, Category = "Cavrnus|Import")
    FOnCavrnusImportComplete OnImportComplete;

    virtual bool CanImport(const FString& FileExtension) const PURE_VIRTUAL(UCavrnusFileImporter::CanImport, return false;);

    // Blueprint-facing wrapper with red pins
    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Import File", AutoCreateRefTerm = "OnStatusUpdate,OnComplete"), Category = "Cavrnus|Import")
    void ImportFile(
        const FString& FilePath,
        const FCavrnusImportSettings& Settings,
        FOnImportStatusUpdateDynamic OnStatusUpdate,
        FOnImportCompleteDynamic OnComplete);

    UFUNCTION(BlueprintCallable, Category = "Cavrnus|Import")
    void CancelImport();

    bool bImportCancelled = false;

    // Subclasses override this
    virtual void ImportFileInternal(const FString& FilePath, const FCavrnusImportSettings& Settings) PURE_VIRTUAL(UCavrnusFileImporter::ImportFileInternal, );

    bool ValidateSourceFileOrBroadcastFailure(const FString& InFilePath);

protected:
    void BroadcastStatus(const FCavrnusImportStatus& Status) { OnImportStatusUpdate.Broadcast(Status); }
    void BroadcastComplete(const FCavrnusImportStatus& FinalStatus) { OnImportComplete.Broadcast(FinalStatus); }

    /**
 * Optional override for loader-specific validation. Called after base validation.
 * Return true to continue, false to indicate failure.
 */
    virtual bool AdditionalValidation(const FString& NormalizedFilePath, FString& StatusMessage);

    /**
     * Helper: build & broadcast an import failure using the existing FCavrnusImportStatus payload.
     */
    void BroadcastImportFailure(const FString& Message);

    FString TrackedFilePath;
};
