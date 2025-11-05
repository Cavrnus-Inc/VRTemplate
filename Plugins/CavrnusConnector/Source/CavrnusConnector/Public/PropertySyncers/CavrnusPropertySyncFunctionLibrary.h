// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusPropertySyncer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/ScriptMacros.h"
#include "CavrnusPropertySyncFunctionLibrary.generated.h"

/**
 * Static helper class that exposes property syncers
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusPropertySyncFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, CustomThunk,
		meta = (DefaultToSelf = "Owner",
			HidePin = "Owner",
			DisplayName = "Cavrnus Sync Property",
			ReturnDisplayName = "PropertySync",
			CustomStructureParam = "Property,SyncOptions"),
			Category = "Cavrnus|Property")
	//static void CavrnusSyncProperty(AActor* Owner, const FCavrnusSyncInfo& SyncInfo, FCavrnusSyncOptions SyncOptions, int32 Property);
	static UCavrnusPropertySyncer* CavrnusInternalSyncProperty(AActor* Owner, const FCavrnusSyncInfo& SyncInfo, int32 Property, int32 SyncOptions);

	DECLARE_FUNCTION(execCavrnusInternalSyncProperty);
};
