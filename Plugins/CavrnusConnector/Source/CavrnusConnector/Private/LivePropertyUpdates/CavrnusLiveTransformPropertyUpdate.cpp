// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "LivePropertyUpdates/CavrnusLiveTransformPropertyUpdate.h"
#include "RelayModel/CavrnusVirtualPropertyUpdate.h"
#include "Translation/CavrnusProtoTranslation.h"
#include "Types\CavrnusPropertyValue.h"

UCavrnusLiveTransformPropertyUpdate::UCavrnusLiveTransformPropertyUpdate()
{
}

UCavrnusLiveTransformPropertyUpdate::~UCavrnusLiveTransformPropertyUpdate()
{
}

void UCavrnusLiveTransformPropertyUpdate::Initialize(Cavrnus::CavrnusRelayModel* relayModel, FCavrnusSpaceConnection spaceConn, const FAbsolutePropertyId& propertyId, FTransform value, const FPropertyPostOptions& options)
{
	InitializeGeneric(relayModel, spaceConn, propertyId, Cavrnus::FPropertyValue::TransformPropValue(value), options);
}

void UCavrnusLiveTransformPropertyUpdate::UpdateWithNewData(FTransform value)
{
	UpdateWithNewDataGeneric(Cavrnus::FPropertyValue::TransformPropValue(value));
}

void UCavrnusLiveTransformPropertyUpdate::Finalize(FTransform value)
{
	FinalizeGeneric(Cavrnus::FPropertyValue::TransformPropValue(value));
}
