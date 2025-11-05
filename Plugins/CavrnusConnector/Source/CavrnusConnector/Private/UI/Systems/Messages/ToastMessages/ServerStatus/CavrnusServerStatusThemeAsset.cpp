// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Messages/ToastMessages/ServerStatus/CavrnusServerStatusThemeAsset.h"

FCavrnusServerStatusTheme UCavrnusServerStatusThemeAsset::GetTheme(const ECavrnusConnectionStatusEnum& Status)
{
	for (const FCavrnusServerStatusTheme Style : Styles)
	{
		if (Style.Status == Status)
			return Style;
	}

	return FCavrnusServerStatusTheme();
}