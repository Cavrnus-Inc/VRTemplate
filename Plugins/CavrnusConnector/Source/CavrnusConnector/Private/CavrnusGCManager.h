// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CavrnusConnectorSettings.h"
#include "Types/CavrnusSpaceConnection.h"

class CavrnusGCManager : public FGCObject
{
public:
	static CavrnusGCManager* GetGCManager();

	void TrackItem(UObject* item);
	void UntrackItem(UObject* item);

private:

	static CavrnusGCManager* Instance;

	TMap<TObjectPtr<UObject>, int> TrackedItemsRefCounts;

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		TArray<TObjectPtr<UObject>> Items;
		TrackedItemsRefCounts.GenerateKeyArray(Items);
		Collector.AddReferencedObjects(Items);
	}
	virtual FString GetReferencerName() const override
	{
		return TEXT("CavrnusGCManager");
	}

#pragma endregion
};