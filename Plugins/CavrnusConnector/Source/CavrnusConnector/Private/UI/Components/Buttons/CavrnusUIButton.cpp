// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Components/Buttons/CavrnusUIButton.h"

#include "Components/Button.h"
#include "UI/Components/Buttons/CavrnusButtonState.h"
#include "UI/Styles/Buttons/CavrnusUIButtonStyle.h"

void UCavrnusUIButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button == nullptr)
		return;

	bIsFocusable = true;
	
	if (ButtonStyleData)
	{
		ApplyButtonStyle();
		ApplyContentStyle(ButtonStyleData->GetStyleForState(ECavrnusButtonState::Normal));
	}
	
	Button->OnClicked.AddDynamic(this, &UCavrnusUIButton::ButtonClicked);
	Button->OnPressed.AddDynamic(this, &UCavrnusUIButton::ButtonPressed);
	Button->OnReleased.AddDynamic(this, &UCavrnusUIButton::ButtonReleased);
	Button->OnHovered.AddDynamic(this, &UCavrnusUIButton::ButtonHovered);
	Button->OnUnhovered.AddDynamic(this, &UCavrnusUIButton::ButtonUnhovered);
}

void UCavrnusUIButton::NativeDestruct()
{
	Super::NativeDestruct();

	if (Button == nullptr)
		return;
	
	Button->OnClicked.RemoveDynamic(this, &UCavrnusUIButton::ButtonClicked);
	Button->OnPressed.RemoveDynamic(this, &UCavrnusUIButton::ButtonPressed);
	Button->OnReleased.RemoveDynamic(this, &UCavrnusUIButton::ButtonReleased);
	Button->OnHovered.RemoveDynamic(this, &UCavrnusUIButton::ButtonHovered);
	Button->OnUnhovered.RemoveDynamic(this, &UCavrnusUIButton::ButtonUnhovered);
}

FReply UCavrnusUIButton::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
	HasKeyboardFocus = true;
	if (ButtonStyleData)
		ApplyContentStyle(ButtonStyleData->GetStyleForState(ResolvePriorityButtonVisualState()));

	return FReply::Handled();
}

void UCavrnusUIButton::NativeOnFocusLost(const FFocusEvent& InFocusEvent)
{
	HasKeyboardFocus = false;
	if (ButtonStyleData)
		ApplyContentStyle(ButtonStyleData->GetStyleForState(ResolvePriorityButtonVisualState()));

	Super::NativeOnFocusLost(InFocusEvent);
}

FReply UCavrnusUIButton::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Enter || InKeyEvent.GetKey() == EKeys::SpaceBar)
	{
		ButtonClicked();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UCavrnusUIButton::SetIcon(UTexture2D* IconTexture)
{
}

void UCavrnusUIButton::SetEnabledState(const bool IsEnabled)
{
	if (Button)
		Button->SetIsEnabled(IsEnabled);
	
	IsDisabled = !IsEnabled;
}

void UCavrnusUIButton::SetVisibleState(const bool IsVisible)
{
	if (Button)
		Button->SetVisibility(IsVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}

void UCavrnusUIButton::OverrideButtonStyle(UCavrnusUIButtonStyle* StyleOverride)
{
	ButtonStyleData = StyleOverride;
	ApplyButtonStyle();
}

ECavrnusButtonState UCavrnusUIButton::ResolvePriorityButtonVisualState()
{
	if (IsDisabled)			return ECavrnusButtonState::Disabled;
	if (IsPressed)			return ECavrnusButtonState::Pressed;
	if (IsHovered)			return ECavrnusButtonState::Hovered;
	if (HasKeyboardFocus)	return ECavrnusButtonState::Focused;

	return ECavrnusButtonState::Normal;
}

#if WITH_EDITOR
const FText UCavrnusUIButton::GetPaletteCategory()
{
	return NSLOCTEXT("UMG", "Cavrnus", "Cavrnus");
}
#endif

void UCavrnusUIButton::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	
	if (ButtonStyleData)
	{
		ApplyButtonStyle();
		ApplyContentStyle(ButtonStyleData->GetStyleForState(ECavrnusButtonState::Normal));
	}
}

void UCavrnusUIButton::ApplyButtonStyle()
{
	if (Button && ButtonStyleData)
		Button->SetStyle(ButtonStyleData->ButtonStyle);
}

void UCavrnusUIButton::ApplyContentStyle(const FCavrnusUIButtonStyleState& NewStyle)
{
	// Button's content goes here, text, icons, etc...
}

void UCavrnusUIButton::ButtonClicked()
{
	if (OnButtonClicked.IsBound())
		OnButtonClicked.Broadcast();
}

void UCavrnusUIButton::ButtonPressed()
{
	IsPressed = true;
	ApplyContentStyle(ButtonStyleData->GetStyleForState(ResolvePriorityButtonVisualState()));
	
	if (OnButtonPressed.IsBound())
		OnButtonPressed.Broadcast();
}

void UCavrnusUIButton::ButtonReleased()
{
	IsPressed = false;
	ApplyContentStyle(ButtonStyleData->GetStyleForState(ResolvePriorityButtonVisualState()));

	if (OnButtonReleased.IsBound())
		OnButtonReleased.Broadcast();
}

void UCavrnusUIButton::ButtonHovered()
{
	IsHovered = true;
	ApplyContentStyle(ButtonStyleData->GetStyleForState(ResolvePriorityButtonVisualState()));

	if (OnButtonHovered.IsBound())
		OnButtonHovered.Broadcast(true);
}

void UCavrnusUIButton::ButtonUnhovered()
{
	IsHovered = false;
	ApplyContentStyle(ButtonStyleData->GetStyleForState(ResolvePriorityButtonVisualState()));

	if (OnButtonHovered.IsBound())
		OnButtonHovered.Broadcast(false);
}