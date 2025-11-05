// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "CavrnusBlueprintModule.h"

#define LOCTEXT_NAMESPACE "CavrnusBlueprintModule"

IMPLEMENT_MODULE(FCavrnusBlueprintModule, CavrnusBlueprintModule)
DEFINE_LOG_CATEGORY(LogCavrnusBlueprintModule);

FCavrnusBlueprintModule::FCavrnusBlueprintModule() {}
FCavrnusBlueprintModule::~FCavrnusBlueprintModule() {}


void FCavrnusBlueprintModule::StartupModule()
{
	IModuleInterface::StartupModule();
	
#if WITH_EDITOR
	FCoreDelegates::OnPostEngineInit.AddLambda([]()
		{
		});
#endif
}

void FCavrnusBlueprintModule::ShutdownModule()
{
	IModuleInterface::ShutdownModule();
}

#undef LOCTEXT_NAMESPACE