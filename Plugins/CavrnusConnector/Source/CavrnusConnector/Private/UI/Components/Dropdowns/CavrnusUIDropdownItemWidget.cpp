// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Components/Dropdowns/CavrnusUIDropdownItemWidget.h"

void UCavrnusUIDropdownItemWidget::Setup(const FCavrnusRuntimeDropdownEntry& InData)
{
	Data = InData;
	
	if (DisplayNameTextBlock)
		DisplayNameTextBlock->SetText(FText::FromString(InData.DisplayName));

	if (Checkmark)
		Checkmark->SetVisibility(ESlateVisibility::Collapsed); // just hiding for now
}
