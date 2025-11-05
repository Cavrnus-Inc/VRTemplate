#pragma once

#include "FileImporter/CavrnusFileImporter.h"
#include "GameFramework/Actor.h"
void ACavrnusFileImporter::ImportFile(
    const FString& FilePath,
    const FCavrnusImportSettings& Settings,
    FOnImportStatusUpdateDynamic OnStatusUpdate,
    FOnImportCompleteDynamic OnComplete)
{
    OnImportStatusUpdate.Clear();
    OnImportComplete.Clear();

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

    ImportFileInternal(FilePath, Settings);
}

void ACavrnusFileImporter::CancelImport()
{
    bImportCancelled = true;
}