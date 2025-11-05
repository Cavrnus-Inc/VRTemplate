#include "RelayNativeInterop.h"
#include <stdio.h>
#include "HAL/PlatformProcess.h"

//#define PathToLibrary L"../../Plugins/CavrnusRelayNative/Binaries/CavrnusRelayNative.dll"

typedef int(*relaylib_initialize)(
    const TCHAR* pluginPathStr,
    const TCHAR* applicationIdentStr,
    const TCHAR* applicationVersionStr,
    const TCHAR* engineId,
    const TCHAR* engineVersion,
    const TCHAR* storagePath,
    bool enableFileLogging,
    bool enableRTC);
relaylib_initialize rl_initialize;

typedef void(__stdcall* cbmsg)(const void* relayRemoteMessageBuf, int buflen);
typedef void(*relaylib_setupcallback)(cbmsg cb);
relaylib_setupcallback rl_setupcallback;

typedef void(__stdcall* cbmsg_passthrough)(const void* passthrough, const void* relayRemoteMessageBuf, int buflen);
typedef void(*relaylib_setupcallback_withpassthrough)(const void* passthrough, cbmsg_passthrough cb);
relaylib_setupcallback_withpassthrough rl_setupcallback_withpassthrough;

typedef void(*relaylib_sendmessage)(const void* relayClientMessageBuf, int buflen);
relaylib_sendmessage rl_sendmessage;

typedef void(*relaylib_shutdown)();
relaylib_shutdown rl_shutdown;

typedef void(*relaylib_test)();
relaylib_test rl_test;

void* handle = NULL;

int relay_loadlibrary(const TCHAR* pluginpath)
{
    if (handle != NULL) // already initialized
    {
        return 0;
    }
 
    handle = FPlatformProcess::GetDllHandle(pluginpath);

    if (handle == NULL)
    {
        return -1;
    }

    rl_initialize = (relaylib_initialize)FPlatformProcess::GetDllExport(handle, L"relay_initialize");
    rl_setupcallback = (relaylib_setupcallback)FPlatformProcess::GetDllExport(handle, L"relay_setupcallback");
    rl_setupcallback_withpassthrough = (relaylib_setupcallback_withpassthrough)FPlatformProcess::GetDllExport(handle, L"relay_setupcallback_withpassthrough");
    rl_sendmessage = (relaylib_sendmessage)FPlatformProcess::GetDllExport(handle, L"relay_sendmessage");
    rl_shutdown = (relaylib_shutdown)FPlatformProcess::GetDllExport(handle, L"relay_shutdown");
    rl_test = (relaylib_test)FPlatformProcess::GetDllExport(handle, L"relay_test");

    return 0;
}

int relay_initialize(
    const TCHAR* pluginpath,
    const TCHAR* applicationIdent,
    const TCHAR* applicationVersion,
    const TCHAR* engineId,
    const TCHAR* engineVersion,
    const TCHAR* storagePath,
    bool enableFileLogging,
    bool enableRTC)
{
    if (handle == NULL)
        return -2;

    int result = rl_initialize(pluginpath, applicationIdent, applicationVersion, engineId, engineVersion, storagePath, enableFileLogging, enableRTC);

    return result;
}

void relay_setupcallback(cbmsg cb)
{
    if (handle == NULL)
        return;

    rl_setupcallback(cb);
}

void relay_setupcallback_withpassthrough(const void* passthrough, cbmsg_passthrough cb)
{
    if (handle == NULL)
        return;

    rl_setupcallback_withpassthrough(passthrough, cb);
}

void relay_sendmessage(const void* buf, int buflen)
{
    if (handle == NULL)
        return;

    rl_sendmessage(buf, buflen);
}

void relay_shutdown()
{
    if (handle == NULL)
        return;

   rl_shutdown();

   // Can we unload the DLL here...
   FPlatformProcess::FreeDllHandle(handle);
   handle = NULL;

   rl_initialize = NULL;
   rl_setupcallback = NULL;
   rl_setupcallback_withpassthrough = NULL;
   rl_sendmessage = NULL;
   rl_shutdown = NULL;
   rl_test = NULL;
}

bool test_fired = false;

void relay_test()
{
    if (handle == NULL)
        return;
    if (test_fired)
        return;
    test_fired = true;

    UE_LOG(LogTemp, Error, TEXT("relay_test"));

    rl_test();
}