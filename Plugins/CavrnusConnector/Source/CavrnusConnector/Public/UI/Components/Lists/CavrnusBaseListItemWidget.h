// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "UI/CavrnusBaseUserWidget.h"
#include "UI/Components/Buttons/CavrnusUIButton.h"
#include "CavrnusBaseListItemWidget.generated.h"

/**
 * Use this widget for list items
 */
UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusBaseListItemWidget : public UCavrnusBaseUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
public:
	TMulticastDelegate<void(UCavrnusBaseListItemWidget* Widget, UObject* DataObject)> OnEntryInitializedDelegate;

	virtual void SetListIndex(int32 Index);
	virtual void SetSelectedState(const bool bState);
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus")
	TObjectPtr<UCavrnusUIButtonStyle> EvenRowButtonTheme;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus")
	TObjectPtr<UCavrnusUIButtonStyle> OddRowButtonTheme;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCavrnusUIButton> Button = nullptr;

	void ListViewSetSelectedItem();
};
