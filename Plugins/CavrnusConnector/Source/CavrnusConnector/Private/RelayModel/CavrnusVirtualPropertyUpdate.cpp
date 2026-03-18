// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "RelayModel/CavrnusVirtualPropertyUpdate.h"
#include "CavrnusConnectorModule.h"
#include "RelayModel/CavrnusRelayModel.h"
#include "Types/CavrnusPropertyValue.h"
#include "Types/AbsolutePropertyId.h"
#include "Translation/CavrnusProtoTranslation.h"

namespace Cavrnus
{
	CavrnusVirtualPropertyUpdate::CavrnusVirtualPropertyUpdate()
	{
	}

	CavrnusVirtualPropertyUpdate::CavrnusVirtualPropertyUpdate(FCavrnusSpaceConnection spaceConn, const FAbsolutePropertyId& propertyId, const FPropertyValue& propVal, const FPropertyPostOptions& options)
	{
		SpaceConn = spaceConn;
		PropertyId = propertyId;
		Options = options;

		LiveUpdaterId = FGuid::NewGuid().ToString();

		lastSentValue = propVal;

		CavrnusRelayModel* relayModel = CavrnusRelayModel::GetDataModel();
		int localChangeId = relayModel->GetSpacePropertyModel(SpaceConn)->SetLocalPropVal(PropertyId, propVal, 1);
		relayModel->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildBeginLivePropertyUpdateMsg(SpaceConn, LiveUpdaterId, PropertyId, propVal, localChangeId, Options));

		lastUpdatedTimeSec = FPlatformTime::Seconds();
	}

	CavrnusVirtualPropertyUpdate::~CavrnusVirtualPropertyUpdate()
	{
	}

	void CavrnusVirtualPropertyUpdate::UpdateWithNewData(const FPropertyValue& propVal)
	{
		if (!CavrnusRelayModel::IsAlive())
		{
			UE_LOG(LogCavrnusConnector, Warning, TEXT("[VirtualPropertyUpdate::UpdateWithNewData] Relay is no longer alive for '%s/%s'. This updater is from a previous space session."), *PropertyId.ContainerName, *PropertyId.PropValueId);
			return;
		}

		lastSentValue = propVal;

		CavrnusRelayModel* relayModel = CavrnusRelayModel::GetDataModel();
		int localChangeId = relayModel->GetSpacePropertyModel(SpaceConn)->SetLocalPropVal(PropertyId, propVal, 1);
		relayModel->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildContinueLivePropertyUpdateMsg(SpaceConn, LiveUpdaterId, PropertyId, propVal, localChangeId, Options));

		lastUpdatedTimeSec = FPlatformTime::Seconds();
	}

	void CavrnusVirtualPropertyUpdate::Finalize()
	{
		if (!CavrnusRelayModel::IsAlive())
		{
			UE_LOG(LogCavrnusConnector, Warning, TEXT("[VirtualPropertyUpdate::Finalize] Relay is no longer alive for '%s/%s'. This updater is from a previous space session."), *PropertyId.ContainerName, *PropertyId.PropValueId);
			return;
		}

		CavrnusRelayModel* relayModel = CavrnusRelayModel::GetDataModel();
		int localChangeId = relayModel->GetSpacePropertyModel(SpaceConn)->SetLocalPropVal(PropertyId, lastSentValue, 1);
		relayModel->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildFinalizeLivePropertyUpdateMsg(SpaceConn, LiveUpdaterId, PropertyId, lastSentValue, localChangeId, Options));

		lastUpdatedTimeSec = FPlatformTime::Seconds();
	}

	void CavrnusVirtualPropertyUpdate::Finalize(const FPropertyValue& propVal)
	{
		if (!CavrnusRelayModel::IsAlive())
		{
			UE_LOG(LogCavrnusConnector, Warning, TEXT("[VirtualPropertyUpdate::Finalize] Relay is no longer alive for '%s/%s'. This updater is from a previous space session."), *PropertyId.ContainerName, *PropertyId.PropValueId);
			return;
		}

		lastSentValue = propVal;

		CavrnusRelayModel* relayModel = CavrnusRelayModel::GetDataModel();
		int localChangeId = relayModel->GetSpacePropertyModel(SpaceConn)->SetLocalPropVal(PropertyId, propVal, 1);
		relayModel->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildFinalizeLivePropertyUpdateMsg(SpaceConn, LiveUpdaterId, PropertyId, propVal, localChangeId, Options));

		lastUpdatedTimeSec = FPlatformTime::Seconds();
	}

	void CavrnusVirtualPropertyUpdate::Cancel()
	{
		if (!CavrnusRelayModel::IsAlive())
		{
			UE_LOG(LogCavrnusConnector, Warning, TEXT("[VirtualPropertyUpdate::Cancel] Relay is no longer alive. This updater is from a previous space session."));
			return;
		}

		CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildCancelLiveUpdateMsg(SpaceConn, LiveUpdaterId));
	}
} // namespace Cavrnus
