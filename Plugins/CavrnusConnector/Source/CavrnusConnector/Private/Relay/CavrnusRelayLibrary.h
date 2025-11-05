// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#ifdef _MSC_VER
#pragma warning(disable : 4251) // needs to have dll-interface to be used by clients of class
#endif

#pragma warning(disable : 4267) // size_t to int
#pragma warning(disable : 4800)	// int to bool
#define GOOGLE_PROTOBUF_INTERNAL_DONATE_STEAL_INLINE 0

#define WIN32_LEAN_AND_MEAN
// Only Windows-specific headers between Allow/Hide
#include "Windows/AllowWindowsPlatformTypes.h"
#include <winsock2.h>
#include <Ws2tcpip.h>
#include "Windows/HideWindowsPlatformTypes.h"

// C++ Standard Library headers come after
#include <string>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <queue>
#include <thread>
#include <functional>
#include <mutex>

// Project-specific headers
#include "Comm/relay.pb.h"
#include "ConnectionStatus.h"
