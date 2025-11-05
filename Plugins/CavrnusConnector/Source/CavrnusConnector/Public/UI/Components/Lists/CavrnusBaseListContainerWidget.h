// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusBaseListItemWidget.h"
#include "CavrnusListViewDataObject.h"
#include "UObject/Interface.h"
#include "CavrnusBaseListContainerWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FCavrnusListItemInitialized, UUserWidget* /* CreatedWidget */, UCavrnusListViewDataObject* /* BoundObject */);
DECLARE_MULTICAST_DELEGATE_OneParam(FCavrnusListItemSelected, UObject* /* SelectedItem */);

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusBaseListContainerWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void InitializeList(TSubclassOf<UCavrnusBaseListItemWidget> EntryWidgetClass);
	virtual void ClearItems();
	
	virtual void AddItemAt(UObject* Item, int32 Index);
	virtual void AddItem(UObject* Item);
	virtual void RemoveItem(UObject* Item);
	virtual void SetSelectedItem(UObject* Item);
	virtual void SetLoadingState(const bool bIsLoading);
	virtual void TeardownList();

	virtual void SetFeedbackText(const FString& InText);

	virtual void ApplySearchFilter(const TArray<UObject*> Filter);

	virtual FCavrnusListItemSelected& OnItemSelected()
	{
		return ItemSelectedDelegate;
	}

	virtual FCavrnusListItemInitialized& OnItemInitialized()
	{
		return ItemInitializedDelegate;
	}

protected:
	FCavrnusListItemSelected ItemSelectedDelegate;
	FCavrnusListItemInitialized ItemInitializedDelegate;

private:
	UPROPERTY()
	TArray<UObject*> TrackedDataObjects;
};
