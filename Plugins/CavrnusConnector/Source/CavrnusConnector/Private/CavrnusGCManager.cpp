// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "CavrnusGCManager.h"

CavrnusGCManager* CavrnusGCManager::Instance = nullptr;

CavrnusGCManager* CavrnusGCManager::GetGCManager()
{
	if (Instance == nullptr)
		Instance = new CavrnusGCManager();
	return Instance;
}

void CavrnusGCManager::TrackItem(UObject* item)
{
	if (!TrackedItemsRefCounts.Contains(item))
		TrackedItemsRefCounts.Add(item, 0);

	TrackedItemsRefCounts[item]++;
}

void CavrnusGCManager::UntrackItem(UObject* item)
{
	//Already untracked
	if (!TrackedItemsRefCounts.Contains(item))
		return;

	TrackedItemsRefCounts[item]--;

	if (TrackedItemsRefCounts[item] <= 0)
		TrackedItemsRefCounts.Remove(item);
}
