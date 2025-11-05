// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Components/Dropdowns/CavrnusUIDropdownWidget.h"

#include "CavrnusConnectorModule.h"
#include "UI/Helpers/CavrnusWidgetFactory.h"

void UCavrnusUIDropdownWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UCavrnusUIDropdownWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

FReply UCavrnusUIDropdownWidget::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
	return Super::NativeOnFocusReceived(InGeometry, InFocusEvent);
}

void UCavrnusUIDropdownWidget::NativeOnFocusLost(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnFocusLost(InFocusEvent);
}

void UCavrnusUIDropdownWidget::Populate(const FCavrnusRuntimeDropdownEntry& Entry)
{
	if (!bHasInitialized)
	{
		ComboBox->OnSelectionChanged.AddDynamic(this, &UCavrnusUIDropdownWidget::HandleSelectionChanged);
		ComboBox->OnGenerateItemWidget.BindUFunction(this, TEXT("HandleGenerateItemWidget"));
		ComboBox->OnGenerateContentWidget.BindUFunction(this, TEXT("HandleGenerateContentWidget"));
		bHasInitialized = true;
	}
	
	if (ComboBox)
	{
		ComboBox->AddOption(Entry.Id);
		EntryMap.Add(Entry.Id, Entry);

		if (Entry.IsDefaultSelected)
			SetSelected(Entry.Id, false);
	}
}

void UCavrnusUIDropdownWidget::Populate(const TArray<FCavrnusRuntimeDropdownEntry>& Entries)
{
	for (auto Entry : Entries)
		Populate(Entry);
}

void UCavrnusUIDropdownWidget::Clear()
{
	if (ComboBox)
		ComboBox->ClearOptions();

	EntryMap.Empty();
}

FCavrnusRuntimeDropdownEntry UCavrnusUIDropdownWidget::GetSelected()
{
	return CurrSelected;
}

void UCavrnusUIDropdownWidget::SetSelected(const FName& Id, const bool bNotify)
{
	if (ComboBox)
		ComboBox->SetSelectedOption(Id);

	if (bNotify)
		NotifySelectionChange(Id);

	if (const FCavrnusRuntimeDropdownEntry* FoundEntryPtr = EntryMap.Find(Id))
		CurrSelected = *FoundEntryPtr;
	else
		UE_LOG(LogCavrnusConnector, Error, TEXT("Unable to SetSelected dropdown item! Has it been added? [%s]"), *Id.ToString());
}

void UCavrnusUIDropdownWidget::SetEnabledState(const bool IsEnabled)
{
	if (ComboBox)
		ComboBox->SetIsEnabled(IsEnabled);
}

void UCavrnusUIDropdownWidget::SetVisibleState(const bool IsVisible)
{
	if (ComboBox)
		ComboBox->SetVisibility(IsVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}

void UCavrnusUIDropdownWidget::HandleSelectionChanged(const FName SelectedKey, ESelectInfo::Type SelectionType)
{
	if (const FCavrnusRuntimeDropdownEntry* FoundEntryPtr = EntryMap.Find(SelectedKey))
	{
		if (FoundEntryPtr)
			FoundEntryPtr->Callback();

		NotifySelectionChange(SelectedKey);

		CurrSelected = *FoundEntryPtr;
	}
}

UWidget* UCavrnusUIDropdownWidget::HandleGenerateItemWidget(const FName ItemKey) const
{
	if (!EntryMap.Contains(ItemKey))
		return nullptr;
	
	if (auto* Widget = FCavrnusWidgetFactory::CreateUserWidget(DropdownItemBlueprint, GetWorld()))
	{
		const FCavrnusRuntimeDropdownEntry& Entry = EntryMap[ItemKey];
		Widget->Setup(Entry);

		return Widget;
	}
	
	return nullptr;	
}

UWidget* UCavrnusUIDropdownWidget::HandleGenerateContentWidget(const FName ItemKey) const
{
	FName KeyToUse = ItemKey;
	if (KeyToUse == NAME_None && ComboBox)
		KeyToUse = ComboBox->GetSelectedOption();

	if (!EntryMap.Contains(KeyToUse))
		return nullptr;

	if (auto* Widget = FCavrnusWidgetFactory::CreateUserWidget(DropdownItemBlueprint, GetWorld()))
	{
		Widget->Setup(EntryMap[KeyToUse]);
		return Widget;
	}

	return nullptr;
}

void UCavrnusUIDropdownWidget::NotifySelectionChange(const FName Id)
{
	if (!EntryMap.Contains(Id))
		return;

	if (OnSelectionChanged.IsBound())
		OnSelectionChanged.Broadcast(EntryMap[Id]);
}
