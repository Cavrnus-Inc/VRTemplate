// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Components/Lists/CavrnusListView.h"
#include "CavrnusConnectorModule.h"

void UCavrnusListView::SetEntryWidgetClass(const TSubclassOf<UUserWidget>& NewEntryWidgetClass)
{
	EntryWidgetClass = NewEntryWidgetClass;
}

void UCavrnusListView::AddItemAt(UObject* Item, const int32 Index)
{
	if (Item == nullptr)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("Cannot add null item into ListView."));
		return;
	}

	if (ListItems.Contains(Item))
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("Cannot add duplicate item into ListView."));
		return;
	}
	
	int32 ClampedIndex = FMath::Clamp(Index, 0, ListItems.Num());
	ListItems.Insert(Item, ClampedIndex);
	
	const TArray<UObject*> Added = { Item };
	const TArray<UObject*> Removed;
	OnItemsChanged(Added, Removed);
	
	RequestRefresh();
}