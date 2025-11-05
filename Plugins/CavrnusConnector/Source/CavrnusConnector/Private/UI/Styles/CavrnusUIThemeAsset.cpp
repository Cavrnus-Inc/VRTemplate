// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Styles/CavrnusUIThemeAsset.h"

FLinearColor UCavrnusUIThemeAsset::GetColorForRole(const ECavrnusThemeColorRole Role) const
{
	if (Role == ECavrnusThemeColorRole::Custom)
		return FLinearColor::White;
	
	return RoleToColor[Role];
}
