#pragma once

#include "CoreMinimal.h"
#include "CavrnusImportDelegates.generated.h"

USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusImportStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Cavrnus | Import")
    float Progress = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Cavrnus | Import")
    FString StatusMessage;

    UPROPERTY(BlueprintReadWrite, Category = "Cavrnus | Import")
    bool bSuccess = false;

    UPROPERTY(BlueprintReadWrite, Category = "Cavrnus | Import")
    TArray<UObject*> ImportedAssets;
};

// Persistent multicast events (BlueprintAssignable)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCavrnusImportStatusUpdate, const FCavrnusImportStatus&, Status);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCavrnusImportComplete, const FCavrnusImportStatus&, FinalStatus);

// Single-cast dynamic delegates for function parameters (red pins)
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnImportStatusUpdateDynamic, const FCavrnusImportStatus&, Status);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnImportCompleteDynamic, const FCavrnusImportStatus&, FinalStatus);

USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusImportSettings
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cavrnus | Import")
    FString TargetFolder;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cavrnus | Import")
    bool bOverwriteExisting = false;
};
