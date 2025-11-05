// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "LivePropertyUpdates/CavrnusLiveStringPropertyUpdate.h"
#include "RelayModel/CavrnusVirtualPropertyUpdate.h"
#include "Translation/CavrnusProtoTranslation.h"

UCavrnusLiveStringPropertyUpdate::UCavrnusLiveStringPropertyUpdate()
{
}

UCavrnusLiveStringPropertyUpdate::~UCavrnusLiveStringPropertyUpdate()
{
}

void UCavrnusLiveStringPropertyUpdate::Initialize(Cavrnus::CavrnusRelayModel* relayModel, FCavrnusSpaceConnection spaceConn, const FAbsolutePropertyId& propertyId, FString value)
{
	InitializeGeneric(relayModel, spaceConn, propertyId, Cavrnus::FPropertyValue::StringPropValue(value));
}

void UCavrnusLiveStringPropertyUpdate::UpdateWithNewData(FString value)
{
	UpdateWithNewDataGeneric(Cavrnus::FPropertyValue::StringPropValue(value));
}

void UCavrnusLiveStringPropertyUpdate::Finalize(FString value)
{
	FinalizeGeneric(Cavrnus::FPropertyValue::StringPropValue(value));
}
