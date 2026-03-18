// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "CavrnusApplication.h"
#include "CavrnusConnector/Public/CavrnusLog.h"
#include "FileImporter/CavrnusLoaderRegistry.h"
#include "FileImporter/CavrnusDatasmithLoader.h"

#define LOCTEXT_NAMESPACE "CavrnusApplication"

IMPLEMENT_MODULE(FCavrnusApplication, CavrnusApplication)
DEFINE_LOG_CATEGORY(LogCavrnusApplication);

FCavrnusApplication::FCavrnusApplication() {}
FCavrnusApplication::~FCavrnusApplication() {}


void FCavrnusApplication::StartupModule()
{
    IModuleInterface::StartupModule();
    UE_LOG(LogCavrnusConnector, Log, TEXT("CavrnusApplication module active."));

    // Inject concrete registry into Connector�s abstract singleton
    FCavrnusLoaderRegistry_Abstract::Set(&FCavrnusLoaderRegistry::Instance());

    // Register concrete loaders
    FCavrnusLoaderRegistry::Instance().RegisterLoader(TEXT("BP_Cavrnus_DatasmithLoader"),
        [](UObject* Outer) { return NewObject<UCavrnusDatasmithLoader>(Outer); });

#if WITH_EDITOR
    FCoreDelegates::OnPostEngineInit.AddLambda([]()
        {
            // Safe place to use Connector APIs
        });
#endif
}

void FCavrnusApplication::ShutdownModule()
{
	IModuleInterface::ShutdownModule();
}

#undef LOCTEXT_NAMESPACE