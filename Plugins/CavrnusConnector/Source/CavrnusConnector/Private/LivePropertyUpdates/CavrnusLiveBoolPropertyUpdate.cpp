// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "LivePropertyUpdates/CavrnusLiveBoolPropertyUpdate.h"
#include "RelayModel/CavrnusVirtualPropertyUpdate.h"
#include "Translation/CavrnusProtoTranslation.h"

UCavrnusLiveBoolPropertyUpdate::UCavrnusLiveBoolPropertyUpdate()
{
}

UCavrnusLiveBoolPropertyUpdate::~UCavrnusLiveBoolPropertyUpdate()
{
}

void UCavrnusLiveBoolPropertyUpdate::Initialize(Cavrnus::CavrnusRelayModel* relayModel, FCavrnusSpaceConnection spaceConn, const FAbsolutePropertyId& propertyId, bool value)
{
	InitializeGeneric(relayModel, spaceConn, propertyId, Cavrnus::FPropertyValue::BoolPropValue(value));
}

void UCavrnusLiveBoolPropertyUpdate::UpdateWithNewData(bool value)
{
	UpdateWithNewDataGeneric(Cavrnus::FPropertyValue::BoolPropValue(value));
}

void UCavrnusLiveBoolPropertyUpdate::Finalize(bool value)
{
	FinalizeGeneric(Cavrnus::FPropertyValue::BoolPropValue(value));
}
