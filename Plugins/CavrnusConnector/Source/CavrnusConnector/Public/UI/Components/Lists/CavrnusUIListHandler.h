// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusBaseListContainerWidget.h"
#include "CavrnusListViewDataObject.h"

template <typename DataType>
class TCavrnusUIListHandler
{
public:
	static TUniquePtr<TCavrnusUIListHandler> Initialize(
		UCavrnusBaseListContainerWidget* InContainer,
		const TSubclassOf<UCavrnusBaseListItemWidget>& InWidgetBlueprint,
		const TFunction<FString(const DataType& Data)> InKeyFunc,
		const TFunction<bool(const DataType& A, const DataType& B)> InSortPredicate)
	{
		auto Handler = MakeUnique<TCavrnusUIListHandler>();
		Handler->InitializeInternal(InContainer, InWidgetBlueprint, InKeyFunc, InSortPredicate);
		return Handler;
	}
	
	void AddItems(const TArray<DataType> Items)
	{
		for (const auto Item : Items)
			AddItem(Item);
	}

	void AddItem(const DataType& Item)
	{
		// this is a bit of a bottleneck...
		DataArray.Add(Item);
		// UE_LOG(LogTemp, Error, TEXT("[AddItem] Count: %d"), SortedDataArray.Num());
	
		auto Index = DataArray.Num() - 1;

		// If we provided a sorter, then sort and grab the resulting sorted index
		if (SortPredicate)
		{
			DataArray.Sort(SortPredicate);
			// Lets grab the index so we can insert to the proper position
			Index = FindElementIndex(Item);
			if (Index == INDEX_NONE)
			{
				UE_LOG(LogTemp, Error, TEXT("INDEX_NONE! Cannot AddItem!"));
				return;
			}
		}

		const FString Key = KeyFunc(Item);
		
		auto DataObj = NewObject<UCavrnusListViewDataObject>();
		DataObj->Id = Key;

		DataMap.Add(Key, Item);
		ObjectMap.Add(Key, DataObj);
		Container->AddItemAt(DataObj, Index);
		ObjectArray.Add(DataObj);
	}

	void UpdateItem(const DataType& Item)
	{
		RemoveItem(Item);
		AddItem(Item);
	}

	void RemoveItem(const DataType& Item)
	{
		const FString Key = KeyFunc(Item);
		if (UObject** FoundDataObjectPtr = ObjectMap.Find(Key))
		{
			Container->RemoveItem(*FoundDataObjectPtr);
			ObjectMap.Remove(Key);
			DataMap.Remove(Key);
			DataArray.Remove(Item);
			ObjectArray.Remove(*FoundDataObjectPtr);
		
			UE_LOG(LogTemp, Error, TEXT("[RemoveItem] Removing Item! AFTER Count: %d"), DataArray.Num());
		}
	}

	void ApplySearchFilter(const FString& SearchText, TFunction<bool(const FString& SearchText, const DataType& Target)> SearchPredicate)
	{
		TArray<DataType> FilteredData;

		for (const DataType& Item : DataArray)
		{
			if (SearchText.IsEmpty() || SearchPredicate(SearchText, Item))
			{
				FilteredData.Add(Item);
			}
		}

		Container->ClearItems();
		for (const DataType& Item : FilteredData)
		{
			Container->AddItem(ObjectMap[KeyFunc(Item)]);
		}
	}
	
	void OnItemSelected(const TFunction<void(const DataType& SelectedData)>& OnSelected)
	{
		UE_LOG(LogTemp, Error, TEXT("[OnItemSelected] Attempting to select item..."));
		
		if (!Container.IsValid() || !OnSelected)
			return;

		for (auto Handle : SelectedHandles)
			Container->OnItemSelected().Remove(Handle);
		
		SelectedHandles.Empty();

		auto Handle = Container->OnItemSelected().AddLambda([this, OnSelected] (const UObject* SelectedObj)
		{
			if (!IsValid(SelectedObj))
			{
				UE_LOG(LogTemp, Error, TEXT("[OnItemSelected] This is a borked SelectedObj..."));
				return;
			}

			if (auto CastedObj = Cast<UCavrnusListViewDataObject>(SelectedObj))
			{
				if (auto FoundData = DataMap.Find(CastedObj->Id))
					OnSelected(*FoundData);
				else
					UE_LOG(LogTemp, Error, TEXT("[OnItemSelected] Unable to find requested data!!..."));
			} else
			{
				UE_LOG(LogTemp, Error, TEXT("[OnItemSelected] Unable to cast!!..."));
			}
		});

		SelectedHandles.Add(Handle);
	}

	template<typename WidgetType>
	void OnWidgetCreated(const TFunction<void(WidgetType* Widget, DataType Data)>& OnCreatedWidget)
	{
		if (Container.IsValid())
		{
			Container->OnItemInitialized().AddLambda(
				[OnCreatedWidget, this](UUserWidget* UserWidget, UCavrnusListViewDataObject* CavrnusListViewDataObject)
				{
					if (CavrnusListViewDataObject == nullptr)
						return;
					
					if (auto* FoundData = DataMap.Find(CavrnusListViewDataObject->Id))
					{
						if (auto* CastedWidget = Cast<WidgetType>(UserWidget))
						{
							OnCreatedWidget(CastedWidget, *FoundData);
						}
					}
				});
		}
	}
	
	void ClearItems()
	{
		for (const auto Item : ObjectMap)
			Container->RemoveItem(Item.Value);

		ObjectMap.Empty();
		DataMap.Empty();
		DataArray.Empty();
	}

	void SetFeedbackText(const FString& InText)
	{
		if (Container.IsValid())
			Container->SetFeedbackText(InText);
	}

	void SetLoadingState(const bool bLoading)
	{
		if (Container.IsValid())
			Container->SetLoadingState(bLoading);
	}

	void Teardown()
	{
		if (Container.IsValid())
		{
			Container->OnItemInitialized().Clear();
			Container->OnItemSelected().Clear();
		}

		for (auto Handle : SelectedHandles)
		{
			if (Container.IsValid())
				Container->OnItemSelected().Remove(Handle);
			Handle.Reset();
		}

		ClearItems();

		OnPopulatedCallback.Clear();

		KeyFunc = nullptr;
		Container = nullptr;
		SortPredicate = nullptr;
		OnWidgetCreatedCallback = nullptr;
	}

	void BindListHasPopulated(const TFunction<void()>& InOnPopulatedCallback)
	{
		if (HasPopulatedAtLeastOneItem)
		{
			InOnPopulatedCallback();
			return;
		}

		OnPopulatedCallback.AddLambda(InOnPopulatedCallback);
	}

	void SetSelectedItem(DataType Item);

private:
	TMulticastDelegate<void(UCavrnusBaseListItemWidget* Widget, UObject* DataObject)> OnEntryInitializedDelegate;
	
	TArray<FDelegateHandle> SelectedHandles;
	
	TArray<DataType> DataArray;
	TMap<FString, DataType> DataMap;
	TMap<FString, UObject*> ObjectMap;
	TArray<UObject*> ObjectArray;

	bool HasPopulatedAtLeastOneItem = false;
	TMulticastDelegate<void()> OnPopulatedCallback;
	
	// Widget data
	TWeakObjectPtr<UCavrnusBaseListContainerWidget> Container = nullptr;
	
	TSubclassOf<UCavrnusBaseListItemWidget> UserWidgetBlueprint;

	// Sort
	TFunction<FString(const DataType&)> KeyFunc;
	TFunction<bool(const DataType&, const DataType&)> SortPredicate;

	// Callbacks
	TFunction<void(DataType)> OnCreate;
	TFunction<void(UUserWidget*)> OnWidgetCreatedCallback;

	void InitializeInternal(
	UCavrnusBaseListContainerWidget* InContainer,
	const TSubclassOf<UCavrnusBaseListItemWidget>& InWidgetBlueprint,
	const TFunction<FString(const DataType& Data)> InKeyFunc,
	const TFunction<bool(const DataType& A, const DataType& B)> InSortPredicate)
	{
		Container = InContainer;
		UserWidgetBlueprint = InWidgetBlueprint;
		KeyFunc = InKeyFunc;
		SortPredicate = InSortPredicate;

		if (Container.IsValid())
		{
			Container->InitializeList(InWidgetBlueprint);

			Container->OnItemInitialized().AddLambda(
				[this](UUserWidget*, UCavrnusListViewDataObject*)
				{
					if (OnPopulatedCallback.IsBound())
						OnPopulatedCallback.Broadcast();
				});
		}
	}
	
	bool Equals(DataType A, DataType B)
	{
		return KeyFunc(A) == KeyFunc(B);
	}

	int32 FindElementIndex(const DataType& Item)
	{
		const FString Key = KeyFunc(Item);
		const int32 Index = DataArray.IndexOfByPredicate([&](const DataType& Elem) {
			 return KeyFunc(Elem) == Key;
		 });

		return Index;
	}
};


template <typename DataType>
void TCavrnusUIListHandler<DataType>::SetSelectedItem(DataType Item)
{
	const FString Key = KeyFunc(Item);

	if (UObject** FoundObjPtr = ObjectMap.Find(Key))
	{
		if (IsValid(*FoundObjPtr))
		{
			Container->SetSelectedItem(*FoundObjPtr);
		}
	}
	else
		UE_LOG(LogTemp, Error, TEXT("[SetSelectedItem] Can't find item!"));
}
