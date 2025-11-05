// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "RelayNetRunner.h"

namespace Cavrnus
{
	static const int PORT_DEFAULT = 36045;
	static const std::string PATH_RELAY_NET_DEFAULT("..\\..\\..\\CavrnusPlugin\\Plugins\\CavrnusConnector\\Source\\ThirdParty\\Collab\\Win64\\Collab.exe");

	RelayNetRunner::RelayNetRunner()
	{
	}

	RelayNetRunner::~RelayNetRunner()
	{
		stopService();
	}

	bool RelayNetRunner::IsRelayNetProcessRunning() const
	{
#if PLATFORM_WINDOWS
		return ProcessHandle.IsValid();
#elif
		return false;
#endif
	}

	void RelayNetRunner::startService(int Port, bool bSilent, const FString& ExecutablePath, const FString& OptionalParameters)
	{
#if PLATFORM_WINDOWS
		FString args = FString::FromInt(Port) + ((OptionalParameters == "") ? "" : " " + OptionalParameters);

		ProcessHandle = FPlatformProcess::CreateProc(*ExecutablePath, *args, false , bSilent, bSilent, nullptr, 0, nullptr, nullptr);
#endif
	}

	void RelayNetRunner::stopService()
	{
#if PLATFORM_WINDOWS
		if(ProcessHandle.IsValid())
			FPlatformProcess::CloseProc(ProcessHandle);
#endif
	}

} // namespace Cavrnus
