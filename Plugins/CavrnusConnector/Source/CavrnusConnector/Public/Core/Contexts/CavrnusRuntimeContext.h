// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusContextBase.h"
#include "Helpers/CavrnusPixelStreamingManager.h"
#include "AssetManager/CavrnusDataAssetManager.h"
#include "Managers/Login/CavrnusLoginManager.h"
#include "Modes/CavrnusModeManager.h"
#include "Pawns/CavrnusPawnManager.h"
#include "PropertySyncers/CavrnusPropertySyncManager.h"
#include "UI/CavrnusUISystems.h"
#include "UObject/Object.h"
#include "Managers/SpawnedObjects/SpawnedObjectsManager.h"
#include "CavrnusRuntimeContext.generated.h"

class UCavrnusDataAssetManager;

// Forward declaration for optional CVT module service
class UCavrnusCVTToolbarButtonRegistry;

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusRuntimeContext : public UCavrnusContextBase
{
	GENERATED_BODY()
public:
	virtual void Initialize(UWorld* World = nullptr);
	
	/**
	 * Gets the service locator for optional service registration.
	 * Allows modules to register their own optional services.
	 */
	UCavrnusServiceLocator* GetServiceLocator() const { return Services; }
};
