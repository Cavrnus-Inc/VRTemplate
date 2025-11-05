// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Styles/Common/CavrnusUICommonStyle.h"

FCavrnusUICommonStyleState UCavrnusUICommonStyle::GetStyleForState(const bool IsHovered)
{
	return IsHovered ? Hovered : Normal;
}