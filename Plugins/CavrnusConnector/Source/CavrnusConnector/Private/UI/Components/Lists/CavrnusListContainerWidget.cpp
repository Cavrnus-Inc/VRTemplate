// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Components/Lists/CavrnusListContainerWidget.h"
#include "CavrnusConnectorModule.h"
#include "UI/Helpers/CavrnusUIScheduler.h"

void UCavrnusListContainerWidget::SetFeedbackText(const FString& InText)
{
	Super::SetFeedbackText(InText);
	if (FeedbackTextBlock)
		FeedbackTextBlock->SetText(FText::FromString(InText));
}

FCavrnusListItemSelected& UCavrnusListContainerWidget::OnItemSelected()
{
	return Super::OnItemSelected();
}

FCavrnusListItemInitialized& UCavrnusListContainerWidget::OnItemInitialized()
{
	return Super::OnItemInitialized();
}

void UCavrnusListContainerWidget::InitializeList(const TSubclassOf<UCavrnusBaseListItemWidget> EntryWidgetClass)
{
	Super::InitializeList(EntryWidgetClass);
	
	if (ListView == nullptr)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("ListContainer is not valid! Cannot add item."));
		return;
	}

	ListView->OnItemClicked().Clear();
	ListView->OnEntryWidgetGenerated().Clear();

	SetLoadingState(false);
	
	if (ListView)
	{
		ListView->OnItemSelectedDelegate.AddWeakLambda(this,[this](UObject* Object)
		{
			if (ItemSelectedDelegate.IsBound())
				ItemSelectedDelegate.Broadcast(Object);
		});

		ListView->OnEntryWidgetGenerated().AddWeakLambda(this, [this](UUserWidget& UserWidget)
		{
			TrackedWidgets.Add(&UserWidget);
		});

		ListView->OnEntryInitializedDelegate.AddWeakLambda(
			this,
			[this](UCavrnusBaseListItemWidget* CavrnusBaseListItemWidget, UObject* DataObject)
		{
				if (auto* CastedObject = Cast<UCavrnusListViewDataObject>(DataObject))
				{
					if (ItemInitializedDelegate.IsBound())
						ItemInitializedDelegate.Broadcast(CavrnusBaseListItemWidget, CastedObject);
				}
		});
	}

	// SelectionMode needs to be set in order for SelectionChanged Delegate to work
	ListView->SetSelectionMode(ESelectionMode::Single);
	ListView->SetEntryWidgetClass(EntryWidgetClass);

	// Handle selection change, mainly for visual states
	ListView->OnItemSelectionChanged().AddWeakLambda(this, [this] (UObject* SelectedObj)
	{
		SetSelectedItem(SelectedObj);
	});
}

void UCavrnusListContainerWidget::AddItemAt(UObject* Item, const int32 Index)
{
	Super::AddItemAt(Item, Index);
	
	if (ListView == nullptr)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("ListContainer is noCavrnusConnectoralid! Cannot add item."));
		return;
	}

	ListView->AddItemAt(Item, Index);
}

void UCavrnusListContainerWidget::AddItem(UObject* Item)
{
	Super::AddItem(Item);
	
	if (ListView == nullptr)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("ListContainer is noCavrnusConnectoralid! Cannot add item."));
		return;
	}

	if (!Item)
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("Attempted to add CavrnusConnectorull widget to the list."));
		return;
	}
	
	ListView->AddItem(Item);
}

void UCavrnusListContainerWidget::RemoveItem(UObject* Item)
{
	Super::RemoveItem(Item);
	
	if (ListView == nullptr)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("ListContainer is noCavrnusConnectoralid! Cannot RemoveItem."));
		return;
	}

	if (!Item)
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("Attempted to add CavrnusConnectorull widget to the list."));
		return;
	}
	
	ListView->RemoveItem(Item);
}

void UCavrnusListContainerWidget::ClearItems()
{
	Super::ClearItems();
	
	if (ListView)
		ListView->ClearListItems();
}

void UCavrnusListContainerWidget::SetSelectedItem(UObject* Item)
{
	// Waiting 2 frames...because well, if this is called when setting up the list, the listView hasn't created any objects yet
	FCavrnusUIScheduler::WaitOneFrame(GetWorld(),[this, Item]
	{
		FCavrnusUIScheduler::WaitOneFrame(GetWorld(), [this, Item]
		{
			if (ListView)
			{
				// Deselect previous
				if (LastSelectedItem && LastSelectedItem != Item)
				{
					if (UCavrnusBaseListItemWidget* PrevWidget = Cast<UCavrnusBaseListItemWidget>(ListView->GetEntryWidgetFromItem(LastSelectedItem)))
						PrevWidget->SetSelectedState(false);
				}

				// Select current
				if (UCavrnusBaseListItemWidget* CurrentWidget = Cast<UCavrnusBaseListItemWidget>(ListView->GetEntryWidgetFromItem(Item)))
					CurrentWidget->SetSelectedState(true);

				LastSelectedItem = Item;
			}
		});
	});
}

void UCavrnusListContainerWidget::TeardownList()
{
}

void UCavrnusListContainerWidget::ApplySearchFilter(const TArray<UObject*> Filter)
{
	Super::ApplySearchFilter(Filter);

	if (ListView)
		ListView->SetListItems(Filter);
}

void UCavrnusListContainerWidget::SetLoadingState(const bool bIsLoading)
{
	Super::SetLoadingState(bIsLoading);

	if (LoaderContainer)
		LoaderContainer->SetVisibility(bIsLoading ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	if (FeedbackTextBlock)
		FeedbackTextBlock->SetVisibility(bIsLoading ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}

void UCavrnusListContainerWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	SetLoadingState(false);
}
