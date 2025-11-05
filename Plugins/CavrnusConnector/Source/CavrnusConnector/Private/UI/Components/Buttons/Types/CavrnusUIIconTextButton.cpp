// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Components/Buttons/Types/CavrnusUIIconTextButton.h"

#include "UI/Components/Image/CavrnusUIImage.h"
#include "UI/Components/Text/CavrnusUITextBlock.h"

void UCavrnusUIIconTextButton::SetButtonText(const FText& NewText)
{
	if (TextBlock)
		TextBlock->SetText(NewText.IsEmpty() ? FText::GetEmpty() : NewText);
}

void UCavrnusUIIconTextButton::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (Icon && ButtonImage)
		Icon->SetBrushFromTexture(ButtonImage);

	SetButtonText(ButtonText);
}

void UCavrnusUIIconTextButton::ApplyContentStyle(const FCavrnusUIButtonStyleState& NewStyle)
{
	Super::ApplyContentStyle(NewStyle);
	
	if (Icon)
		Icon->SetColorAndOpacity(NewStyle.ContentColor);

	if (TextBlock)
		TextBlock->SetColorAndOpacity(NewStyle.ContentColor);
}