// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"

class FCavrnusContentBrowserExtender
{
public:
	static void Register();
	static void Unregister();

private:
	static TSharedRef<FExtender> OnExtendAssetViewContextMenu(const TArray<FAssetData>& SelectedAssets);
	static void BuildCavrnusMenuEntries(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);

	static FDelegateHandle ExtenderDelegateHandle;
};
