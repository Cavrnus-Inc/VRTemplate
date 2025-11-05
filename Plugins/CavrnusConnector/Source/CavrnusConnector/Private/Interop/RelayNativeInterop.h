#pragma once

#include "CoreMinimal.h"
#include <stdio.h>

extern "C" {

int relay_loadlibrary(const TCHAR* pluginpath);

int relay_initialize(
	const TCHAR* pluginPathStr,
    const TCHAR* applicationIdentStr,
    const TCHAR* applicationVersionStr,
    const TCHAR* engineId,
    const TCHAR* engineVersion,
    const TCHAR* storagePath,
    bool enableFileLogging,
    bool enableRTC);

typedef void(__stdcall* cbmsg)(const void* relayRemoteMessageBuf, int buflen);
void relay_setupcallback(cbmsg cb);

typedef void(__stdcall* cbmsg_passthrough)(const void* passthrough, const void* relayRemoteMessageBuf, int buflen);
void relay_setupcallback_withpassthrough(const void* passthrough, cbmsg_passthrough cb);

void relay_sendmessage(const void* relayClientMessageBuf, int buflen);

void relay_shutdown();

void relay_test();

}
