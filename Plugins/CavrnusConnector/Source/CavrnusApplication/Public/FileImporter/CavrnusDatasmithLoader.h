#pragma once
#include "CoreMinimal.h"
#include "CavrnusBaseLoader.h"
#include "DatasmithRuntime.h"
#include "Types/CavrnusPropertyValue.h"
#include "DatasmithFileImporter.h" // make sure this is included at the top
#include "FileImporter/DatasmithUtilities.h"
#include "CavrnusDatasmithLoader.generated.h"

UCLASS()
class CAVRNUSAPPLICATION_API UCavrnusDatasmithLoader : public UCavrnusBaseLoader
{
    GENERATED_BODY()

public:
    virtual bool CanHandleId_Implementation(const FString& WellKnownObjectId) const override
    {
        return WellKnownObjectId.Equals(TEXT("BP_Cavrnus_DatasmithLoader"));
    }
    virtual void BeginDestroy() override;
protected:
    virtual void DoLoadInternal(const FCavrnusSpawnedObject& ObjectData, UWorld* World) override;

private:
    FString ContainerName = "";
    FString FileInfoPropertyName = "DatasmithFile";

    void HandlePropertyChanged(const Cavrnus::FPropertyValue& Value, const FString& Container, const FString& PropertyName);
    void BeginImport(const FString& FileID, const FString& FileName);

    void MapCacheTopLevel(const FString& UniqueId, const TArray<FString>& ExtractedFiles);

    void NormalizeExtractedDependencies(
        const FString& UniqueId,
        const FString& BaseName,
        const TArray<FString>& ExtractedFiles,
        TMap<FString, FDatasmithMaterialMetadata>& InMaterialMap,
        TMap<FString, FDatasmithMeshSlotMetadata>& InMeshSlotMap);

private:
    UPROPERTY()
    FString LastImportedFilePath;

    EDatasmithRuntimeFileType LastImportedFileType = EDatasmithRuntimeFileType::Unknown;

    UFUNCTION() 
    void OnImporterStatus(const FCavrnusImportStatus& Status);
    UFUNCTION() 
    void OnImporterComplete(const FCavrnusImportStatus& FinalStatus);

    UPROPERTY() // keep alive, prevent GC
    UDatasmithFileImporter* ActiveImporter = nullptr;

    TMap<FString, FDatasmithMeshSlotMetadata> MeshSlotMap;
    TMap<FString, FDatasmithMaterialMetadata> MaterialMap;
};