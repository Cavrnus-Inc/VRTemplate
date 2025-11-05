// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Components/Text/CavrnusUITextBlock.h"

#include "Styling/SlateTypes.h"
#include "UI/Styles/CavrnusUIThemeManager.h"
#include "UI/Styles/Text/CavrnusUIFontInfo.h"
#include "UI/Styles/Text/CavrnusUITextStyle.h"

void UCavrnusUITextBlock::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (Style == nullptr)
		return;

	FTextBlockStyle ModifiedStyle = Style->TextStyle;

	const int32 FontSize = FCavrnusUIFontInfo::GetFontSizeForTextBlockSize(TextSize);
	ModifiedStyle.Font.Size = FontSize;

	if (ColorRole != ECavrnusThemeColorRole::Custom)
	{
		if (const UCavrnusUIThemeAsset* Theme = UCavrnusUIThemeManager::GetTheme(this))
		{
			const FLinearColor Color = Theme->GetColorForRole(ColorRole);
			ModifiedStyle.ColorAndOpacity = Color;
			SetColorAndOpacity(Color);
		}
	}
	
	SetFont(ModifiedStyle.Font);
}

#if WITH_EDITOR
const FText UCavrnusUITextBlock::GetPaletteCategory()
{
	return NSLOCTEXT("UMG", "Cavrnus", "Cavrnus");
}
#endif
