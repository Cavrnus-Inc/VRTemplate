#pragma once

#include "FileImporter/CavrnusImportDelegates.h"
#include "GameFramework/Actor.h"
#include "CavrnusFileImporter.generated.h"

UCLASS(Abstract, Blueprintable)
class CAVRNUSCONNECTOR_API ACavrnusFileImporter : public AActor
{
    GENERATED_BODY()

public:
    // Persistent multicast events
    UPROPERTY(BlueprintAssignable, Category = "Cavrnus | Import")
    FOnCavrnusImportStatusUpdate OnImportStatusUpdate;

    UPROPERTY(BlueprintAssignable, Category = "Cavrnus | Import")
    FOnCavrnusImportComplete OnImportComplete;

    virtual bool CanImport(const FString& FileExtension) const PURE_VIRTUAL(ACavrnusFileImporter::CanImport, return false;);

    // Blueprint-facing wrapper with red pins
    UFUNCTION(BlueprintCallable, meta = (DisplayName = "Import File", AutoCreateRefTerm = "OnStatusUpdate,OnComplete"), Category = "Cavrnus | Import")
    void ImportFile(
        const FString& FilePath,
        const FCavrnusImportSettings& Settings,
        FOnImportStatusUpdateDynamic OnStatusUpdate,
        FOnImportCompleteDynamic OnComplete);

    UFUNCTION(BlueprintCallable, Category = "Cavrnus | Import")
    void CancelImport();

    bool bImportCancelled = false;

    // Subclasses override this
    virtual void ImportFileInternal(const FString& FilePath, const FCavrnusImportSettings& Settings) PURE_VIRTUAL(ACavrnusFileImporter::ImportFileInternal, );
};
