// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Messages/ToastMessages/Info/CavrnusInfoToastMessageThemeAsset.h"

FCavrnusInfoToastMessageTheme UCavrnusInfoToastMessageThemeAsset::GetTheme(const ECavrnusInfoToastMessageEnum& Status)
{
	for (const FCavrnusInfoToastMessageTheme Style : Styles)
	{
		if (Style.Status == Status)
			return Style;
	}

	return FCavrnusInfoToastMessageTheme();
}
