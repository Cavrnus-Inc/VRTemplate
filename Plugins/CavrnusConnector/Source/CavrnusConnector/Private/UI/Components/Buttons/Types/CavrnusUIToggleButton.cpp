// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Components/Buttons/Types/CavrnusUIToggleButton.h"
#include "Components/Button.h"
#include "UI/Components/Buttons/CavrnusButtonState.h"

void UCavrnusUIToggleButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button)
		Button->OnClicked.AddDynamic(this, &UCavrnusUIToggleButton::ToggleButtonClicked);
}

void UCavrnusUIToggleButton::NativeDestruct()
{
	Super::NativeDestruct();

	if (Button)
		Button->OnClicked.RemoveDynamic(this, &UCavrnusUIToggleButton::ToggleButtonClicked);

}

void UCavrnusUIToggleButton::ApplyButtonStyle()
{
	if (Button && ButtonStyleData)
		Button->SetStyle(IsToggled ? ButtonStyleData->ToggledStyle : ButtonStyleData->ButtonStyle);
}

void UCavrnusUIToggleButton::InitToggleSetting(const TSharedPtr<TSetting<bool>>& InSetting)
{
	ToggleSetting = InSetting;
	ToggleSetting.Get()->Bind(this, [this](const bool& Val)
	{
		SetToggled(Val, false);
	});
}

void UCavrnusUIToggleButton::SetCanToggleFunc(const TFunction<bool()>& CanToggleFunc)
{
	ToggleFunc = CanToggleFunc;
}

void UCavrnusUIToggleButton::SetToggled(const bool InToggled, const bool bNotify)
{
	IsToggled = InToggled;

	if (bNotify)
	{
		if (OnToggleChanged.IsBound())
			OnToggleChanged.Broadcast(InToggled);
	}

	ApplyButtonStyle();
}

bool UCavrnusUIToggleButton::GetIsToggled() const
{
	return IsToggled;
}

void UCavrnusUIToggleButton::SetIcon(UTexture2D* IconTexture)
{
	Super::SetIcon(IconTexture);

	if (ButtonImage && IconTexture)
		ButtonImage->SetBrushFromTexture(IconTexture);
}

ECavrnusButtonState UCavrnusUIToggleButton::ResolvePriorityButtonVisualState()
{
	if (IsToggled)  return ECavrnusButtonState::Toggled;
	if (IsDisabled) return ECavrnusButtonState::Disabled;

	return ECavrnusButtonState::Normal;
}

void UCavrnusUIToggleButton::ToggleButtonClicked()
{
	if (ToggleFunc && ToggleFunc())
		SetToggled(!IsToggled);
}
