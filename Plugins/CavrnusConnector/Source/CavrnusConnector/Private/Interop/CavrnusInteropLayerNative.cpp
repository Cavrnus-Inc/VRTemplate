// Copyright (c) 2024 Cavrnus. All rights reserved.

#include "CavrnusInteropLayerNative.h"

#include "CavrnusConnectorSettings.h"
#include "CavrnusConnector/Public/CavrnusSubsystem.h"
#include <HAL/FileManager.h>
#include <Interfaces/IPluginManager.h>
#include <Misc/Paths.h>
#include "Misc/App.h"
#include "Misc/EngineVersion.h"
#include "RelayNativeInterop.h"

#define PERFORMANCE_TRACKING 0

namespace Cavrnus
{
	void cavrnusInteropLayerNativeCallbackTrampoline(const void* ciln, const void* buf, int buflen)
	{
		CavrnusInteropLayerNative* l = (CavrnusInteropLayerNative*)ciln;
		l->ReceiveMessageCallback(buf, buflen);
	}

	//===============================================================
	CavrnusInteropLayerNative::CavrnusInteropLayerNative()
		: sendbuf(NULL), sendbuflen(0), messageProcessingQueuePhase(false)
	{
	}

	//===============================================================
	void CavrnusInteropLayerNative::Start()
	{
		static FString PluginPath = IPluginManager::Get().FindPlugin("CavrnusConnector")->GetBaseDir();
		FString BinaryLocation = PluginPath / "Source" / "ThirdParty" / "Collab" / "win-x64" / "Collab.dll";

		UE_LOG(LogTemp, Warning, TEXT("Plugin location: %s"), *BinaryLocation);

		int r = relay_loadlibrary(*BinaryLocation);
		if (r < 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load Relay Native Library: %d"), r)
		}

		relay_setupcallback_withpassthrough((const void*)this, cavrnusInteropLayerNativeCallbackTrampoline);

		FString BinaryPath = PluginPath / "Source" / "ThirdParty" / "Collab" / "win-x64";

		// TODO Pass user information to relay here to set up app names, version, logging preferences, etc.
		const TCHAR* projName = FApp::GetProjectName();
		const TCHAR* buildVersion = FApp::GetBuildVersion();
		const TCHAR* engineId = TEXT("unreal");
		const TCHAR* engineVersion = *FEngineVersion::Current().ToString();
		const TCHAR* storagePath = *UCavrnusSubsystem::Get()->Services->Get<UCavrnusConnectorSettings>()->StorageRootPath;
		const bool enableLogging = UCavrnusSubsystem::Get()->Services->Get<UCavrnusConnectorSettings>()->LogOutputToFile;
		const bool disableRtc = UCavrnusSubsystem::Get()->Services->Get<UCavrnusConnectorSettings>()->EnableRTC;
		
		r = relay_initialize(*BinaryPath, projName, buildVersion, engineId, engineVersion, storagePath, enableLogging, disableRtc);
		if (r < 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to initialize Relay Native: %d"), r)
		}
	}

	//===============================================================
	CavrnusInteropLayerNative::~CavrnusInteropLayerNative()
	{
		LogPerformance();

		relay_shutdown();

		if (sendbuf != NULL)
			delete[] sendbuf;
		sendbuf = NULL;
		sendbuflen = 0;
	}

	//===============================================================
	void CavrnusInteropLayerNative::ReceiveMessageCallback(const void* buf, int len)
	{
		ServerData::RelayRemoteMessage* msg = new ServerData::RelayRemoteMessage();
		if (!msg->ParseFromArray(buf, len)) // failed if false
		{
			delete msg;
			return;
		}

		if (messageProcessingQueuePhase)
			MessageProcessingQueueA_.push(msg);
		else
			MessageProcessingQueueB_.push(msg);
	}


	//===============================================================
	void CavrnusInteropLayerNative::LogPerformance()
	{
#if PERFORMANCE_TRACKING
		int totalEntries = 0;
		int totalMessages = 0;
		int maxMessagesPerSecond = 0;
		float avgMessagesPerSecond = 0;

		for (std::map<int, int>::iterator it = PropertiesSentPerSecond.begin(); it != PropertiesSentPerSecond.end(); it++)
		{
			totalEntries++;
			totalMessages += it->second;
			maxMessagesPerSecond = it->second > maxMessagesPerSecond ? it->second : maxMessagesPerSecond;
			avgMessagesPerSecond = (float)(totalMessages) / (float)(totalEntries);
		}
		UE_LOG(LogCavrnusConnector, Log, TEXT("Total number of messages: %d"), totalMessages);
		UE_LOG(LogCavrnusConnector, Log, TEXT("Total number of 1 second time windows: %d"), totalEntries);
		UE_LOG(LogCavrnusConnector, Log, TEXT("Max messages per second: %d"), maxMessagesPerSecond);
		UE_LOG(LogCavrnusConnector, Log, TEXT("Average messages per second: %f"), avgMessagesPerSecond);
#endif
	}

	
	//===============================================================
	void CavrnusInteropLayerNative::SendMessage(const ServerData::RelayClientMessage& message)
	{
		int messageLength = message.ByteSizeLong();
		if (messageLength > sendbuflen)
		{
			if (sendbuf != NULL)
				delete[] sendbuf;
			sendbuf = NULL;
			sendbuflen = 1024*1024;
			while (sendbuflen < messageLength)
				sendbuflen *= 2;
			sendbuf = new uint8[sendbuflen];
		}
		bool bSerializeResult = message.SerializeToArray(sendbuf, messageLength);
		if (!bSerializeResult)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to serialize relay message."));
			return;
		}
		
		relay_sendmessage(sendbuf, messageLength);
	}

	void CavrnusInteropLayerNative::ReceiveMessages(ICavrnusInteropMessageReceiver* receiver)
	{
		messageProcessingQueuePhase = !messageProcessingQueuePhase; // flip buffers

		MessageProcessingQueue& recvqueue = messageProcessingQueuePhase ? MessageProcessingQueueB_ : MessageProcessingQueueA_;
				
		while (!recvqueue.empty())
		{
			ServerData::RelayRemoteMessage* msg = recvqueue.front();
			recvqueue.pop();
			receiver->ReceiveMessage(*msg);
			delete msg;
    	}		
	}
	
	//===============================================================
	void CavrnusInteropLayerNative::RunService()
	{

	}

	//TODO: QUESTION, can't this just be done synchronously in SendMessage?
	void CavrnusInteropLayerNative::DoTick()
	{
	}

} // namespace Cavrnus
