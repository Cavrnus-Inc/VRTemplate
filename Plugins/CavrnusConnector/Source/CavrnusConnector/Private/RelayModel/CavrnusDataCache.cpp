// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "RelayModel/CavrnusDataCache.h"
#include "RelayModel/CavrnusRelayModel.h"
#include "Translation/CavrnusProtoTranslation.h"

namespace Cavrnus
{
	CavrnusDataCache::CavrnusDataCache()
	{
	}

	CavrnusDataCache::~CavrnusDataCache()
	{

	}

	void CavrnusDataCache::Setup(TArray<FString> keys, TArray<FString> vals)
	{
		for (int i = 0; i < keys.Num(); i++) {
			StringValues.Add(keys[i], vals[i]);
		}
	}

	FString CavrnusDataCache::GetStringValue(FString key)
	{
		if (StringValues.Contains(key))
			return StringValues[key];
		else
			return "";
	}

	void CavrnusDataCache::SetStringValue(FString key, FString val)
	{
		if (StringValues.Contains(key))
			StringValues[key] = val;
		else
			StringValues.Add(key, val);

		Cavrnus::CavrnusRelayModel::GetDataModel()->SendMessage(Cavrnus::CavrnusProtoTranslation::BuildPostDataCacheUpdate(key, val));
	}
} // namespace Cavrnus
