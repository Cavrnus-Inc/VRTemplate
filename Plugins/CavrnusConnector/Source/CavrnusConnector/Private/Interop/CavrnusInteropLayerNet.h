// Copyright (c) 2024 Cavrnus. All rights reserved.

#pragma once

#include "Relay/RelayClient.h"
#include "Relay/RelayNetRunner.h"

#include <thread>

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "CavrnusInteropLayer.h"

namespace Cavrnus
{
	class CavrnusInteropLayerNet : public CavrnusInteropLayer
	{
	public:
		CavrnusInteropLayerNet(const FString& serverIP, int serverPort);
		virtual ~CavrnusInteropLayerNet();

		virtual void Start();

		virtual void SendMessage(const ServerData::RelayClientMessage& message);
		
		virtual void ReceiveMessages(ICavrnusInteropMessageReceiver* receiver);

		virtual void DoTick();

		virtual void RunService();

		virtual void LogPerformance();

	private:
		void SendLoop();
		void ReceiveLoop();
		ServerData::RelayClientMessage BuildKeepAlive();
		void SetStatus(ConnectionStatus newStatus);
		const FString GetPluginPath();


		std::atomic<bool>	ServiceIsStarted;
		std::atomic<int>	CurrTick;
		std::atomic<int>	LastKeepAliveTick;

		RelayClient						Client_;
		std::atomic<ConnectionStatus>   Status_;

		std::atomic<bool>				Stop_;
		mutable std::mutex				Send_mutex_;
		mutable std::mutex				Receive_mutex_;

		std::thread						SendThread_;
		std::thread						ReceiveThread_;

		Cavrnus::RelayNetRunner			RelayNetRunner_;

		ServerData::RelayClientMessageBatch SendMessageBatch;

		typedef std::queue<std::shared_ptr<ServerData::RelayRemoteMessage> > MessageProcessingQueue;
		MessageProcessingQueue MessageProcessingQueue_;

		TArray<ServerData::RelayClientMessage> QueuedSendMessages;


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
