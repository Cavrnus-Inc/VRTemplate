// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "LivePropertyUpdates/CavrnusLiveColorPropertyUpdate.h"
#include "RelayModel/CavrnusVirtualPropertyUpdate.h"
#include "Translation/CavrnusProtoTranslation.h"

UCavrnusLiveColorPropertyUpdate::UCavrnusLiveColorPropertyUpdate()
{
}

UCavrnusLiveColorPropertyUpdate::~UCavrnusLiveColorPropertyUpdate()
{
}

void UCavrnusLiveColorPropertyUpdate::Initialize(Cavrnus::CavrnusRelayModel* relayModel, FCavrnusSpaceConnection spaceConn, const FAbsolutePropertyId& propertyId, FLinearColor value)
{
	InitializeGeneric(relayModel, spaceConn, propertyId, Cavrnus::FPropertyValue::ColorPropValue(value));
}

void UCavrnusLiveColorPropertyUpdate::UpdateWithNewData(FLinearColor value)
{
	UpdateWithNewDataGeneric(Cavrnus::FPropertyValue::ColorPropValue(value));
}

void UCavrnusLiveColorPropertyUpdate::Finalize(FLinearColor value)
{
	FinalizeGeneric(Cavrnus::FPropertyValue::ColorPropValue(value));
}
