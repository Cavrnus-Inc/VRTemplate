#pragma once

#include "UObject/Object.h"
#include "CavrnusFunctionLibrary.h"
#include "Managers/SpawnedObjects/CavrnusImportDelegates.h"
#include "Delegates/Delegate.h"
#include "FileImporter/CavrnusPump.h"

bool Worker_UnzipAll(const FString& ZipPath,
    const FString& DestFolder,
    TArray<FString>& OutFiles,
    FString& OutError,
    TFunction<void(const FString& FileName, float Progress)> OnFileExtracted);
