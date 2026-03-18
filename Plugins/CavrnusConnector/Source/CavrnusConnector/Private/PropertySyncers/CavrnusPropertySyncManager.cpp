// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "PropertySyncers/CavrnusPropertySyncManager.h"

void UCavrnusPropertySyncManager::Initialize()
{
	Super::Initialize();
	Tracked.Empty();
}

void UCavrnusPropertySyncManager::Register(const FGuid& Id, UCavrnusPropertySyncer* PropertySync)
{
	Tracked.Add(Id, PropertySync);
}

void UCavrnusPropertySyncManager::Unregister(const FGuid& Id)
{
	Tracked.Remove(Id);
}

void UCavrnusPropertySyncManager::Dispose()
{
	Super::Dispose();

	if (Tracked.IsEmpty())
		return;
	
	for (const TPair<FGuid, UCavrnusPropertySyncer*>& Pair : Tracked)
	{
		if (IsValid(Pair.Value))
			Pair.Value->Dispose();
	}

	Tracked.Empty();
}
