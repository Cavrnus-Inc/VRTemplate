// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Messages/ToastMessages/Info/CavrnusInfoToastMessageWidget.h"

void UCavrnusInfoToastMessageWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UCavrnusInfoToastMessageWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	if (ThemeAsset)
		SetStyle(ToastMessageEnum);
}

UCavrnusBaseToastMessageWidget* UCavrnusInfoToastMessageWidget::SetType(const ECavrnusInfoToastMessageEnum& InType)
{
	ToastMessageEnum = InType;
	SetStyle(InType);

	return this;
}

void UCavrnusInfoToastMessageWidget::SetStyle(const ECavrnusInfoToastMessageEnum& InType)
{
	if (ThemeAsset)
	{
		const auto Style = ThemeAsset->GetTheme(InType);

		if (PrimaryBorder)
			PrimaryBorder->SetBrushColor(Style.PrimaryColor);

		if (Icon)
			Icon->SetBrushFromTexture(Style.Icon);
	}
}
