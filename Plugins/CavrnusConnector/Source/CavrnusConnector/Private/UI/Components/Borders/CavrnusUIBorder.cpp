// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Components/Borders/CavrnusUIBorder.h"

#include "UI/Styles/CavrnusUIThemeManager.h"

void UCavrnusUIBorder::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (const UCavrnusUIThemeAsset* Theme = UCavrnusUIThemeManager::GetTheme(this))
	{
		const FLinearColor Color = Theme->GetColorForRole(ColorRole);
		SetBrushColor(Color);
	}
}

#if WITH_EDITOR
const FText UCavrnusUIBorder::GetPaletteCategory()
{
	return NSLOCTEXT("UMG", "Cavrnus", "Cavrnus");
}
#endif