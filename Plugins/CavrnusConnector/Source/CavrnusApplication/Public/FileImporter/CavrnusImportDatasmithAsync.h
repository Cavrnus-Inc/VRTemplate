#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"
#include "Managers/SpawnedObjects/CavrnusImportDelegates.h"
#include "DatasmithFileImporter.h"
#include "CavrnusImportDatasmithAsync.generated.h"

UCLASS()
class CAVRNUSAPPLICATION_API UCavrnusImportDatasmithAsync : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    // Entry point for Blueprint
    UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly="true", WorldContext="WorldContextObject"), Category="Cavrnus|Import")
    static UCavrnusImportDatasmithAsync* ImportDatasmithFile(
        UObject* WorldContextObject,
        const FString& FilePath,
        const FCavrnusImportSettings& Settings);

    // Delegates exposed to Blueprint
    UPROPERTY(BlueprintAssignable)
    FOnCavrnusImportStatusUpdate OnStatusUpdate;

    UPROPERTY(BlueprintAssignable)
    FOnCavrnusImportComplete OnComplete;

    // UBlueprintAsyncActionBase override
    virtual void Activate() override;

private:
    // Internal state
    UPROPERTY()
    UObject* WorldContextObject;

    FString FilePath;
    FCavrnusImportSettings Settings;

    UPROPERTY()
    UDatasmithFileImporter* Importer;

    // Callbacks
    UFUNCTION()
    void HandleStatus(const FCavrnusImportStatus& Status);

    UFUNCTION()
    void HandleComplete(const FCavrnusImportStatus& FinalStatus);
};
