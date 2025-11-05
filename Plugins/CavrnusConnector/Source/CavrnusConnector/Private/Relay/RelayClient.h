// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CavrnusRelayLibrary.h"
#include <Containers/UnrealString.h>

class FSocket;

namespace Cavrnus
{
	class RelayClient
	{
	public:
		RelayClient(const FString& ServerAddressIn, int ServerPortIn);
		virtual ~RelayClient();

		bool CreateListenSocket();
		bool Connect();

		bool SendMessage(const ServerData::RelayClientMessageBatch& message);
		bool ReceiveMessage(ServerData::RelayRemoteMessageBatch& message);

		void Disconnect();

		int GetServerPort() const;

	private:
		void HandleSocketError();

		FString ServerAddress;
		int ServerPort;

		FSocket* ListenSocket;
		FSocket* ClientSocket;
	};
} // namespace Cavrnus
