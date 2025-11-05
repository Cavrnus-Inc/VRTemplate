// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusRuntimeDropdownEntry.h"
#include "CavrnusUIDropdownItemWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/ComboBoxKey.h"
#include "CavrnusUIDropdownWidget.generated.h"

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusUIDropdownWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnFocusLost(const FFocusEvent& InFocusEvent) override;
	
public:
	TMulticastDelegate<void(const FCavrnusRuntimeDropdownEntry& SelectedEntry)> OnSelectionChanged;
	
	void Populate(const FCavrnusRuntimeDropdownEntry& Entry);
	void Populate(const TArray<FCavrnusRuntimeDropdownEntry>& Entries);
	void Clear();

	FCavrnusRuntimeDropdownEntry GetSelected();
	void SetSelected(const FName& Id, bool bNotify = false);
	
	virtual void SetEnabledState(bool IsEnabled);
	virtual void SetVisibleState(bool IsVisible);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UComboBoxKey> ComboBox = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus")
	TSubclassOf<UCavrnusUIDropdownItemWidget> DropdownItemBlueprint = nullptr;

private:
	FCavrnusRuntimeDropdownEntry CurrSelected = FCavrnusRuntimeDropdownEntry();
	
	bool bHasInitialized = false;
	
	UPROPERTY()
	TMap<FName, FCavrnusRuntimeDropdownEntry> EntryMap;

	UFUNCTION()
	void HandleSelectionChanged(FName SelectedKey, ESelectInfo::Type SelectionType);
	
	UFUNCTION()
	UWidget* HandleGenerateItemWidget(FName ItemKey) const;

	UFUNCTION()
	UWidget* HandleGenerateContentWidget(FName ItemKey) const;

	void NotifySelectionChange(const FName Id);
};
