// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusRuntimeDropdownEntry.generated.h"

USTRUCT()
struct FCavrnusRuntimeDropdownEntry
{
	GENERATED_BODY()

	FName Id;
	FString DisplayName;
	bool IsDefaultSelected = false;
	TFunction<void()> Callback;

	FCavrnusRuntimeDropdownEntry() {}
	FCavrnusRuntimeDropdownEntry(const FName& InId, const FString& InDisplayName, const TFunction<void()>& InCallback)
	{
		Id = InId;
		DisplayName = InDisplayName;
		Callback = InCallback;
	}

	static FCavrnusRuntimeDropdownEntry Create(const FName& InId, const FString& InDisplayName, const bool bIsSelected, const TFunction<void()>& InCallback)
	{
		FCavrnusRuntimeDropdownEntry Entry;
		Entry.Id = InId;
		Entry.DisplayName = InDisplayName;
		Entry.IsDefaultSelected = bIsSelected;
		Entry.Callback = InCallback;
		return Entry;
	}
};
