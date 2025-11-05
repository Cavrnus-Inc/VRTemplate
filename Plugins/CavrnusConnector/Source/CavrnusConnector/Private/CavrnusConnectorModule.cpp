// Copyright (c) 2025 Cavrnus. All rights reserved.

// Includes
#include "CavrnusConnectorModule.h"
#include <Misc/Paths.h>

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

}

//===============================================================
void FCavrnusConnectorModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE
