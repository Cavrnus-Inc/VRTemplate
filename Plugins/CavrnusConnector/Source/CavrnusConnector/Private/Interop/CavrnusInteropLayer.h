// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "Relay/RelayClient.h"

#include <thread>

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

namespace Cavrnus
{
	class ICavrnusInteropMessageReceiver
	{
	public:
		virtual void ReceiveMessage(const ServerData::RelayRemoteMessage& msg) = 0;
	};
	
	class CavrnusInteropLayer
	{
	public:
		CavrnusInteropLayer() {}
		virtual ~CavrnusInteropLayer() {}

		virtual void Start() = 0;

		virtual void SendMessage(const ServerData::RelayClientMessage& message) = 0;

		virtual void ReceiveMessages(ICavrnusInteropMessageReceiver* receiver) = 0;

		virtual void DoTick() = 0;

		virtual void RunService() = 0;

		virtual void LogPerformance() = 0;
	};
} // namespace Cavrnus
