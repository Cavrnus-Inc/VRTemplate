// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Components/Lists/CavrnusBaseListContainerWidget.h"

void UCavrnusBaseListContainerWidget::InitializeList(TSubclassOf<UCavrnusBaseListItemWidget> EntryWidgetClass)
{
	TrackedDataObjects.Empty();
}

void UCavrnusBaseListContainerWidget::ClearItems()
{
}

void UCavrnusBaseListContainerWidget::AddItemAt(UObject* Item, int32 Index)
{
	TrackedDataObjects.Add(Item);
}

void UCavrnusBaseListContainerWidget::AddItem(UObject* Item)
{
	TrackedDataObjects.Add(Item);
}

void UCavrnusBaseListContainerWidget::RemoveItem(UObject* Item)
{
	TrackedDataObjects.Remove(Item);
}

void UCavrnusBaseListContainerWidget::SetSelectedItem(UObject* Item)
{
}

void UCavrnusBaseListContainerWidget::SetLoadingState(const bool bIsLoading)
{
}

void UCavrnusBaseListContainerWidget::TeardownList()
{
	TrackedDataObjects.Empty();
}

void UCavrnusBaseListContainerWidget::SetFeedbackText(const FString& InText)
{
}

void UCavrnusBaseListContainerWidget::ApplySearchFilter(const TArray<UObject*> Filter)
{
}
