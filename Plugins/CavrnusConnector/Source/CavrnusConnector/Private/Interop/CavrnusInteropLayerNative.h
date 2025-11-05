// Copyright (c) 2024 Cavrnus. All rights reserved.

#pragma once

#include "CavrnusInteropLayer.h"

namespace Cavrnus
{
	class CavrnusInteropLayerNative : public CavrnusInteropLayer
	{
	public:
		CavrnusInteropLayerNative();
		virtual ~CavrnusInteropLayerNative();

		virtual void Start();

		virtual void SendMessage(const ServerData::RelayClientMessage& message);
		
		virtual void ReceiveMessages(ICavrnusInteropMessageReceiver* receiver);

		virtual void DoTick();

		virtual void RunService();

		virtual void LogPerformance();

		// This shouldn't be public, but needs to be called from the static trampoline.
		void ReceiveMessageCallback(const void* buf, int len);

	private:
		const FString GetPluginPath();

		uint8* sendbuf;
		int sendbuflen;

		typedef std::queue<ServerData::RelayRemoteMessage*> MessageProcessingQueue;
		MessageProcessingQueue MessageProcessingQueueA_;
		MessageProcessingQueue MessageProcessingQueueB_;
		bool messageProcessingQueuePhase;

		//Copied from: https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c
		std::string random_string(size_t length)
		{
			auto randchar = []() -> char
				{
					const char charset[] =
						"0123456789"
						"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
						"abcdefghijklmnopqrstuvwxyz";
					const size_t max_index = (sizeof(charset) - 1);
					return charset[rand() % max_index];
				};
			std::string str(length, 0);
			std::generate_n(str.begin(), length, randchar);
			return str;
		}

		std::map<int, int> PropertiesSentPerSecond;
	};
} // namespace Cavrnus

