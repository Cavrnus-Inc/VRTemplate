// Copyright (c) 2025 Cavrnus. All rights reserved.

// Includes
#include "CavrnusConnectorModule.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"

#include "CavrnusRelayLibrary.h"
#include "RelayModel/CavrnusRelayModel.h"

#define LOCTEXT_NAMESPACE "CavrnusConnectorModule"
DEFINE_LOG_CATEGORY(LogCavrnusConnector);
IMPLEMENT_MODULE(FCavrnusConnectorModule, CavrnusConnector)

//===============================================================
FCavrnusConnectorModule::FCavrnusConnectorModule()
{
}

//===============================================================
FCavrnusConnectorModule::~FCavrnusConnectorModule()
{
}


//===============================================================
void FCavrnusConnectorModule::StartupModule()
{
	UE_LOG(LogCavrnusConnector, Log, TEXT("CavrnusConnector module active."));

	TArray<FModuleStatus> ModuleStatuses;
	FModuleManager::Get().QueryModules(ModuleStatuses);

	TArray<FString> LoadedCavrnusModules;
	for (const FModuleStatus& Status : ModuleStatuses)
	{
		FString ModuleName = Status.Name;
		if (Status.bIsLoaded && ModuleName.StartsWith(TEXT("Cavrnus")))
		{
			LoadedCavrnusModules.Add(ModuleName);
		}
	}
	LoadedCavrnusModules.Sort();

	FString ModuleList = LoadedCavrnusModules.Num() > 0
		? FString::Join(LoadedCavrnusModules, TEXT(", "))
		: TEXT("(none loaded yet)");
	UE_LOG(LogCavrnusConnector, Log, TEXT("Cavrnus Connector plugin active. Loaded modules: %s"), *ModuleList);
}

//===============================================================
void FCavrnusConnectorModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE
