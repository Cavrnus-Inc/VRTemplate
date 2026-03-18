// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "DataAssets/CavrnusCVTToolbarButtonConfigAsset.h"
#include "Algo/Sort.h"

int32 UCavrnusCVTToolbarButtonConfigAsset::GetInsertIndexForButton(const FString& ButtonName) const
{
	for (const FCavrnusToolbarButtonConfig& Config : ButtonConfigs)
	{
		if (Config.ButtonName == ButtonName && Config.bEnabled)
		{
			return Config.InsertIndex;
		}
	}
	return INDEX_NONE;
}

bool UCavrnusCVTToolbarButtonConfigAsset::IsButtonEnabled(const FString& ButtonName) const
{
	for (const FCavrnusToolbarButtonConfig& Config : ButtonConfigs)
	{
		if (Config.ButtonName == ButtonName)
		{
			return Config.bEnabled;
		}
	}
	return false;
}

TArray<FString> UCavrnusCVTToolbarButtonConfigAsset::GetEnabledButtonNames() const
{
	TArray<FString> EnabledButtons;
	
	// First, collect all enabled buttons with their indices
	TArray<TPair<int32, FString>> ButtonIndexPairs;
	
	for (const FCavrnusToolbarButtonConfig& Config : ButtonConfigs)
	{
		if (Config.bEnabled)
		{
			// Use a large number for INDEX_NONE to sort them to the end
			int32 SortIndex = (Config.InsertIndex == INDEX_NONE) ? MAX_int32 : Config.InsertIndex;
			ButtonIndexPairs.Add(TPair<int32, FString>(SortIndex, Config.ButtonName));
		}
	}
	
	// Sort by insert index
	Algo::Sort(ButtonIndexPairs, [](const TPair<int32, FString>& A, const TPair<int32, FString>& B)
	{
		return A.Key < B.Key;
	});
	
	// Extract button names in sorted order
	for (const TPair<int32, FString>& Pair : ButtonIndexPairs)
	{
		EnabledButtons.Add(Pair.Value);
	}
	
	return EnabledButtons;
}

