// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusBaseListContainerWidget.h"
#include "CavrnusListView.h"
#include "UI/Components/Text/CavrnusUITextBlock.h"
#include "CavrnusListContainerWidget.generated.h"

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusListContainerWidget : public UCavrnusBaseListContainerWidget
{
	GENERATED_BODY()
public:
	UPROPERTY()
	UObject* LastSelectedItem = nullptr;
	
	virtual void SetFeedbackText(const FString& InText) override;
	virtual FCavrnusListItemSelected& OnItemSelected() override;
	virtual FCavrnusListItemInitialized& OnItemInitialized() override;
	virtual void InitializeList(TSubclassOf<UCavrnusBaseListItemWidget> EntryWidgetClass) override;
	virtual void AddItemAt(UObject* Item, int32 Index) override;
	virtual void AddItem(UObject* Item) override;
	virtual void RemoveItem(UObject* Item) override;
	virtual void ClearItems() override;
	virtual void SetSelectedItem(UObject* Item) override;
	virtual void TeardownList() override;
	virtual void ApplySearchFilter(const TArray<UObject*> Filter) override;
	virtual void SetLoadingState(const bool bIsLoading) override;

protected:
	virtual void NativePreConstruct() override;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusListView> ListView = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUITextBlock> FeedbackTextBlock = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UPanelWidget> LoaderContainer = nullptr;

private:
	UPROPERTY()
	TArray<UUserWidget*> TrackedWidgets;
};
