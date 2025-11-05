// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Components/Buttons/Types/CavrnusUITextButton.h"

#include "UI/Components/Text/CavrnusUITextBlock.h"

void UCavrnusUITextButton::SetButtonText(const FText& NewText)
{
	LocallySetText = NewText.ToString();
	
	if (TextBlock)
		TextBlock->SetText(NewText.IsEmpty() ? FText::GetEmpty() : NewText);
}

void UCavrnusUITextButton::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	const FText TextToUse = !LocallySetText.IsEmpty() ? FText::FromString(LocallySetText) : ButtonText;
	SetButtonText(TextToUse);
}

void UCavrnusUITextButton::ApplyContentStyle(const FCavrnusUIButtonStyleState& NewStyle)
{
	Super::ApplyContentStyle(NewStyle);

	// This overrides the text block color...
	if (TextBlock)
		TextBlock->SetColorAndOpacity(NewStyle.ContentColor);
}