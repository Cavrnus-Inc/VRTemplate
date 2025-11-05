// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CavrnusRelayLibrary.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatformProcess.h"
#endif

namespace Cavrnus
{
	class RelayNetRunner
	{
	public:
		RelayNetRunner();
		virtual ~RelayNetRunner();

		void startService(int Port, bool bSilent, const FString& ExecutablePath, const FString& OptionalParameters);
		void stopService();
		bool IsRelayNetProcessRunning() const;

	private:
#if PLATFORM_WINDOWS
		FProcHandle ProcessHandle;
#endif
	};
} // namespace Cavrnus
