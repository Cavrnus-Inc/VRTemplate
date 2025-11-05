// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Styles/Buttons/CavrnusUIButtonStyle.h"
#include "UI/Components/Buttons/CavrnusButtonState.h"

const FCavrnusUIButtonStyleState& UCavrnusUIButtonStyle::GetStyleForState(const ECavrnusButtonState State) const
{
	switch (State)
	{
	case ECavrnusButtonState::Hovered:
		return Hovered;
	case ECavrnusButtonState::Pressed:
		return Pressed;
	case ECavrnusButtonState::Disabled:
		return Disabled;
	case ECavrnusButtonState::Focused:
		return Focused;
	case ECavrnusButtonState::Toggled:
		return Focused;
	default:
		return Normal;
	}
}
