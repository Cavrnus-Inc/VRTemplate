// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Components/Lists/CavrnusBaseListItemWidget.h"
#include "UI/Components/Lists/CavrnusListView.h"

void UCavrnusBaseListItemWidget::SetListIndex(const int32 Index)
{
	if (Button == nullptr)
		return;

	Button->OverrideButtonStyle(Index % 2 == 0 ? EvenRowButtonTheme : OddRowButtonTheme);
}

void UCavrnusBaseListItemWidget::SetSelectedState(const bool bState)
{
	
}

void UCavrnusBaseListItemWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	if (const UCavrnusListView* OwningListView = Cast<UCavrnusListView>(GetOwningListView()))
	{
		const int32 Index = OwningListView->GetListItems().Find(ListItemObject);
		SetListIndex(Index);
		
		if (OwningListView->OnEntryInitializedDelegate.IsBound())
			OwningListView->OnEntryInitializedDelegate.Broadcast(this, ListItemObject);

		if (Button)
		{
			Button->OnButtonClicked.RemoveAll(this);
			Button->OnButtonClicked.AddWeakLambda(this, [this, OwningListView, ListItemObject]
			{
				ListViewSetSelectedItem();
				
				if (OwningListView->OnItemSelectedDelegate.IsBound())
					OwningListView->OnItemSelectedDelegate.Broadcast(ListItemObject);
			});
		}
	}
}

void UCavrnusBaseListItemWidget::ListViewSetSelectedItem()
{
	// No idea of how else to do this...why isn't SetSelectedItem in the base list view class?
	if (UListView* ListView = Cast<UListView>(GetOwningListView()))
		ListView->SetSelectedItem(GetListItem());
}
