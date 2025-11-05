// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Components/Buttons/Types/CavrnusUIIconButton.h"
#include "UI/Components/Image/CavrnusUIImage.h"

void UCavrnusUIIconButton::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (Icon && ButtonImage)
		Icon->SetBrushFromTexture(ButtonImage);
}

void UCavrnusUIIconButton::ApplyContentStyle(const FCavrnusUIButtonStyleState& NewStyle)
{
	Super::ApplyContentStyle(NewStyle);

	if (Icon)
		Icon->SetColorAndOpacity(NewStyle.ContentColor);
}
