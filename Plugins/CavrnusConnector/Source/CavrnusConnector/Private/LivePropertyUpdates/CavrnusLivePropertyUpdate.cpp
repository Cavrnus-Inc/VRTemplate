// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "LivePropertyUpdates/CavrnusLivePropertyUpdate.h"
#include "RelayModel/CavrnusVirtualPropertyUpdate.h"
#include "Translation/CavrnusProtoTranslation.h"
#include "Types/CavrnusPropertyValue.h"
#include "Types/AbsolutePropertyId.h"

UCavrnusLivePropertyUpdate::UCavrnusLivePropertyUpdate() : livePropertyUpdate(nullptr)
{
}

UCavrnusLivePropertyUpdate::~UCavrnusLivePropertyUpdate()
{
	if (livePropertyUpdate)
	{
		delete livePropertyUpdate;
		livePropertyUpdate = nullptr;
	}
}

void UCavrnusLivePropertyUpdate::InitializeGeneric(Cavrnus::CavrnusRelayModel* relayModel, FCavrnusSpaceConnection spaceConn, const FAbsolutePropertyId& propertyId, Cavrnus::FPropertyValue value, const FPropertyPostOptions& options)
{
	livePropertyUpdate = new Cavrnus::CavrnusVirtualPropertyUpdate(relayModel, spaceConn, propertyId, value, options);
}

void UCavrnusLivePropertyUpdate::Cancel()
{
	if (livePropertyUpdate)
		livePropertyUpdate->Cancel();
}

double UCavrnusLivePropertyUpdate::GetLastUpdatedTimeSeconds()
{
	if (livePropertyUpdate)
		return livePropertyUpdate->lastUpdatedTimeSec;
	
	return 0.0f;
}

void UCavrnusLivePropertyUpdate::UpdateWithNewDataGeneric(const Cavrnus::FPropertyValue& propVal)
{
	if (livePropertyUpdate && !(lastSentPropValue == propVal))
	{
		livePropertyUpdate->UpdateWithNewData(propVal);
		lastSentPropValue = propVal;
	}
}

void UCavrnusLivePropertyUpdate::FinalizeGeneric(const Cavrnus::FPropertyValue& propVal)
{
	if (livePropertyUpdate)
		livePropertyUpdate->Finalize(propVal);
}

void UCavrnusLivePropertyUpdate::FinalizeCurrentValue()
{
	if (livePropertyUpdate)
		livePropertyUpdate->Finalize();
}
