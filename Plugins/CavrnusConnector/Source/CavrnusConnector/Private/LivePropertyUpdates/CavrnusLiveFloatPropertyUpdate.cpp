// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "LivePropertyUpdates/CavrnusLiveFloatPropertyUpdate.h"
#include "RelayModel/CavrnusVirtualPropertyUpdate.h"
#include "Translation/CavrnusProtoTranslation.h"

UCavrnusLiveFloatPropertyUpdate::UCavrnusLiveFloatPropertyUpdate()
{
}

UCavrnusLiveFloatPropertyUpdate::~UCavrnusLiveFloatPropertyUpdate()
{
}

void UCavrnusLiveFloatPropertyUpdate::Initialize(Cavrnus::CavrnusRelayModel* relayModel, FCavrnusSpaceConnection spaceConn, const FAbsolutePropertyId& propertyId, float value)
{
	InitializeGeneric(relayModel, spaceConn, propertyId, Cavrnus::FPropertyValue::FloatPropValue(value));
}

void UCavrnusLiveFloatPropertyUpdate::UpdateWithNewData(float value)
{
	UpdateWithNewDataGeneric(Cavrnus::FPropertyValue::FloatPropValue(value));
}

void UCavrnusLiveFloatPropertyUpdate::Finalize(float value)
{
	FinalizeGeneric(Cavrnus::FPropertyValue::FloatPropValue(value));
}
