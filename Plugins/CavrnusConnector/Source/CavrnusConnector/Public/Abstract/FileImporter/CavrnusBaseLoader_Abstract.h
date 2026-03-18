#pragma once

#include "CoreMinimal.h"
#include "Core/DisposableUObject.h"
#include "Engine/World.h"              // for UWorld
#include "Types/CavrnusSpawnedObject.h"
#include "Managers/SpawnedObjects/CavrnusImportDelegates.h"
#include "CavrnusBaseLoader_Abstract.generated.h"

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusBaseLoader_Abstract : public UDisposableUObject
{
    GENERATED_BODY()

public:
    /** Entry point: subclasses must implement load logic */
    virtual void StartLoad(const FCavrnusSpawnedObject& ObjectData, UWorld* World) PURE_VIRTUAL(UCavrnusBaseLoader_Abstract::StartLoad, );

    /** Optional cancel hook */
    virtual void CancelLoad() {}

    /** Optional finalize hook */
    virtual void FinalizeLoad() {}

    // --- Blueprint dynamic multicast (BlueprintAssignable) ---
    /*
    UPROPERTY(BlueprintAssignable)
    FOnCavrnusImportStatusUpdate OnStatusUpdate;

    UPROPERTY(BlueprintAssignable)
    FOnCavrnusImportComplete OnComplete;
    */

    // --- Native multicast delegates (C++ only) ---
    FOnCavrnusImportStatusUpdateNative OnStatusUpdateNative;
    FOnCavrnusImportCompleteNative OnCompleteNative;
};