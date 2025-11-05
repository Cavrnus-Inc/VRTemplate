// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CavrnusConnectorModule.h"
#include "SpacePropertyModel.h"
#include "Types/CavrnusBinding.h"
#include "Types/CavrnusSpaceConnection.h"

#include <CoreMinimal.h>

namespace Cavrnus
{
	/**
	 * @class CavrnusRelayModel
	 * @brief Manages the relay communication between the Cavrnus server and the client.
	 *
	 * Handles the reception and processing of messages from the Cavrnus server,
	 * as well as sending messages to the server. Also responsible for managing 
	 * data models and invoking callbacks for object creation and destruction.
	 */
	class CAVRNUSCONNECTOR_API CavrnusBindingModel
	{
	public:
		static CavrnusBindingModel* GetBindingModel();

		FString RegisterBinding(CavrnusUnbind bindingCallback);

		void UnbindBinding(FString bindingId);

	private:
		static CavrnusBindingModel* Instance;

		TMap<FString, CavrnusUnbind*> BindingCallbacks;
	};
} // namespace Cavrnus
