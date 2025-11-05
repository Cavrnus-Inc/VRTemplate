// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusBaseListItemWidget.h"
#include "Components/ListView.h"
#include "CavrnusListView.generated.h"

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusListView : public UListView
{
	GENERATED_BODY()
public:
	TMulticastDelegate<void(UObject* DataObject)> OnItemSelectedDelegate;
	TMulticastDelegate<void(UCavrnusBaseListItemWidget* Widget, UObject* DataObject)> OnEntryInitializedDelegate;
	
	void SetEntryWidgetClass(const TSubclassOf<UUserWidget>& NewEntryWidgetClass);
	void AddItemAt(UObject* Item, int32 Index);
};
