// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include <MathUtil.h>
#include "Types/CavrnusPropertyValue.h"
#include "Types/AbsolutePropertyId.h"
#include "Types/PropertyPostOptions.h"

namespace Cavrnus
{
	class CavrnusRelayModel;

	class CavrnusVirtualPropertyUpdate
	{
	public:
		CavrnusVirtualPropertyUpdate();
		CavrnusVirtualPropertyUpdate(Cavrnus::CavrnusRelayModel* relayModel, FCavrnusSpaceConnection spaceConn, const FAbsolutePropertyId& propertyId, const FPropertyValue& propVal, const FPropertyPostOptions& options = FPropertyPostOptions());
		~CavrnusVirtualPropertyUpdate();

		void UpdateWithNewData(const FPropertyValue& propVal);
		void Finalize();
		void Finalize(const FPropertyValue& propVal);

		void Cancel();

		double lastUpdatedTimeSec;

		FPropertyValue lastSentValue;

	protected:
		FCavrnusSpaceConnection SpaceConn;

		FString LiveUpdaterId;

		FAbsolutePropertyId PropertyId;

		Cavrnus::CavrnusRelayModel* RelayModel;

		FPropertyPostOptions Options;
	};
} // namespace Cavrnus
