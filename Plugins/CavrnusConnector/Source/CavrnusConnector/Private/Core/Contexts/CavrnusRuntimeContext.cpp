// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Core/Contexts/CavrnusRuntimeContext.h"
#include "Modules/ModuleManager.h"

void UCavrnusRuntimeContext::Initialize(UWorld* World)
{
	// There may be some order dependencies here (e.g. UISystems requires DataAssetManager)
	Services = NewObject<UCavrnusServiceLocator>(this);
	Services->RegisterService<UCavrnusDataAssetManager>(); // Place at top as other services may be using DataAssets
	Services->RegisterService<UCavrnusUISystems>(World); // UI System depends on DataAssets being available
	Services->RegisterService<UCavrnusLoginManager>();

	Services->RegisterService<UCavrnusModeManager>(); // No obvious dependency between these two
	Services->RegisterService<UCavrnusPawnManager>(); 

	Services->RegisterService<UCavrnusPropertySyncManager>();
	Services->RegisterService<UCavrnusPixelStreamingManager>();
	Services->RegisterService<USpawnedObjectsManager>();

	// Note: Optional CVT toolbar button registry is registered by the CVT module itself
	// in its StartupModule to avoid build order/dependency issues
}
