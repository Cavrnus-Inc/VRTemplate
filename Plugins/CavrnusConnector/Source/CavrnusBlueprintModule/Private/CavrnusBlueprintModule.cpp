// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "CavrnusBlueprintModule.h"
#include "CavrnusConnector/Public/CavrnusLog.h"

#if WITH_EDITOR
#include "EdGraphUtilities.h"
#include "CustomPins/CavrnusSpawnActorPinFactory.h"
#endif

IMPLEMENT_MODULE(FCavrnusBlueprintModule, CavrnusBlueprintModule)
DEFINE_LOG_CATEGORY(LogCavrnusBlueprintModule);

FCavrnusBlueprintModule::FCavrnusBlueprintModule() {}
FCavrnusBlueprintModule::~FCavrnusBlueprintModule() {}

#define LOCTEXT_NAMESPACE "FCavrnusBlueprintModule"

void FCavrnusBlueprintModule::StartupModule()
{
    IModuleInterface::StartupModule();
    UE_LOG(LogCavrnusConnector, Log, TEXT("CavrnusBlueprintModule module active."));

#if WITH_EDITOR
    // Create and register your pin factory
    PinFactory = MakeShareable(new FCavrnusSpawnActorPinFactory());
    FEdGraphUtilities::RegisterVisualPinFactory(PinFactory);
#endif
}

void FCavrnusBlueprintModule::ShutdownModule()
{
    IModuleInterface::ShutdownModule();

#if WITH_EDITOR
    if (PinFactory.IsValid())
    {
        FEdGraphUtilities::UnregisterVisualPinFactory(PinFactory);
        PinFactory.Reset();
    }
#endif
}

#undef LOCTEXT_NAMESPACE