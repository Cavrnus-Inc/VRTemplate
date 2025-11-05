// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "AssetManager/DataAssets/CavrnusPawnSettingsDataAsset.h"
#include "CavrnusConnectorModule.h"

FCavrnusPawnTypeData* UCavrnusPawnSettingsDataAsset::FindPawnById(const FString& PawnId)
{
	for (TPair<FString, FCavrnusPawnTypeData>& Pair : OtherRemotePawns)
	{
		if (Pair.Key.Equals(PawnId, ESearchCase::IgnoreCase))
			return &Pair.Value;
	}

	UE_LOG(LogCavrnusConnector, Warning, TEXT("[FindPawnById] Unable to find PawnData using %s"), *PawnId)
	return nullptr;
}
