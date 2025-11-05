// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include <Containers/Map.h>
#include "Types/CavrnusBinding.h"
#include "Types/CavrnusCallbackTypes.h"

namespace Cavrnus
{
	class CavrnusDataCache
	{
	public:
		CavrnusDataCache();
		virtual ~CavrnusDataCache();

		void Setup(TArray<FString> keys, TArray<FString> vals);

		FString GetStringValue(FString key);
		void SetStringValue(FString key, FString val);

	private:
		TMap<FString, FString> StringValues;
	};
} // namespace Cavrnus
