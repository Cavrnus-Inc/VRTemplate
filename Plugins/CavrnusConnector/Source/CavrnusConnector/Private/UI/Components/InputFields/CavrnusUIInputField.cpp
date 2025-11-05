// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Components/InputFields/CavrnusUIInputField.h"

#include "Components/Border.h"
#include "UI/Components/Borders/CavrnusUIBorder.h"
#include "UI/Components/EditableTextBox/CavrnusUIEditableTextBox.h"
#include "UI/Components/Text/CavrnusUITextBlock.h"
#include "UI/Styles/InputFields/CavrnusInputFieldStyle.h"
#include <Misc/EngineVersionComparison.h>

UCavrnusUIInputField* UCavrnusUIInputField::SetToKeyboardFocusedCustom()
{
	if (EditableTextBox)
		EditableTextBox->SetKeyboardFocus();
	
	return this;
}

UCavrnusUIInputField* UCavrnusUIInputField::SetText(const FString& InText)
{
	if (EditableTextBox)
		EditableTextBox->SetText(FText::FromString(InText));

	return this;
}

UCavrnusUIInputField* UCavrnusUIInputField::SetHintText(const FString& InText)
{
	if (EditableTextBox)
		EditableTextBox->SetHintText(FText::FromString(InText));

	return this;
}

UCavrnusUIInputField* UCavrnusUIInputField::SetIsPasswordField(const bool bIsPassword)
{
	if (EditableTextBox)
		EditableTextBox->SetIsPassword(bIsPassword);
	
	return this;
}

UCavrnusUIInputField* UCavrnusUIInputField::SetLabel(const FString& InText)
{
	if (Label)
		Label->SetText(FText::FromString(InText));

	return this;
}

UCavrnusUIInputField* UCavrnusUIInputField::SetVisibilityState(const bool bVis)
{
	this->SetVisibility(bVis ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	return this;
}

FString UCavrnusUIInputField::GetText()
{
	if (EditableTextBox)
		return EditableTextBox->GetText().ToString();

	return "";
}

void UCavrnusUIInputField::ClearText()
{
	if (EditableTextBox)
		EditableTextBox->SetText(FText::GetEmpty());
}

void UCavrnusUIInputField::ResetField()
{
	if (EditableTextBox)
		EditableTextBox->SetHintText(FText::FromString(HintString));
	
	ApplyCurrentStateStyle();
}

void UCavrnusUIInputField::NativeConstruct()
{
	Super::NativeConstruct();

	if (EditableTextBox)
	{
		EditableTextBox->OnTextChanged.AddDynamic(this, &UCavrnusUIInputField::HandleTextUpdated);
		EditableTextBox->OnTextCommitted.AddDynamic(this, &UCavrnusUIInputField::HandleTextCommitted);
	}
}

void UCavrnusUIInputField::NativeDestruct()
{
	Super::NativeDestruct();
	
	if (EditableTextBox)
	{
		EditableTextBox->OnTextChanged.RemoveDynamic(this, &UCavrnusUIInputField::HandleTextUpdated);
		EditableTextBox->OnTextCommitted.RemoveDynamic(this, &UCavrnusUIInputField::HandleTextCommitted);
	}
}

void UCavrnusUIInputField::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (InputFieldStyleData)
	{
		if (EditableTextBox)
			EditableTextBox->SetHintText(FText::FromString(HintString));

		ApplyCurrentStateStyle();
	}
}

void UCavrnusUIInputField::HandleTextUpdated(const FText& InText)
{
	if (OnTextChanged.IsBound())
		OnTextChanged.Broadcast(InText);
}

void UCavrnusUIInputField::HandleTextCommitted(const FText& InText, ETextCommit::Type InCommitMethod)
{
	if (OnInputCommitted.IsBound())
		OnInputCommitted.Broadcast(InText);
}

void UCavrnusUIInputField::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
}

void UCavrnusUIInputField::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
}

void UCavrnusUIInputField::ApplyHoverStyle(const bool HoverState)
{
	if (EditableTextBox == nullptr)
		return;

	if (InputFieldStyleData == nullptr)
		return;

	const auto ColorValue = HoverState 
									? GetCurrentStateData().NormalInputFieldContentColorString
									: GetCurrentStateData().HoveredInputFieldContentColorString;

	if (Border)
		Border->SetContentColorAndOpacity(ColorValue);
}

void UCavrnusUIInputField::ApplyCurrentStateStyle()
{
	if (!EditableTextBox || !InputFieldStyleData)
		return;

	const FInputFieldStyleStateData CurrentState = GetCurrentStateData();

	auto CreateRoundedBoxBrush = [this](const FLinearColor& BackgroundColor, const FLinearColor& OutlineColor) -> FSlateBrush
	{
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
		Brush.TintColor = FSlateColor(BackgroundColor);
        
		Brush.OutlineSettings.Width = InputFieldStyleData->OutlineWidth;
		Brush.OutlineSettings.CornerRadii = InputFieldStyleData->CornerRadii;
		Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
		Brush.OutlineSettings.Color = FSlateColor(OutlineColor);
        
		return Brush;
	};

	const FSlateBrush NormalBrush = CreateRoundedBoxBrush(CurrentState.NormalBackgroundColor, CurrentState.NormalOutlineColor);
	const FSlateBrush HoveredBrush = CreateRoundedBoxBrush(CurrentState.HoverBackgroundColor, CurrentState.HoverOutlineColor);
	const FSlateBrush FocusedBrush = CreateRoundedBoxBrush(CurrentState.FocusedBackgroundColor, CurrentState.FocusedOutlineColor);

	FEditableTextBoxStyle NewStyle = EditableTextBox->WidgetStyle;
	NewStyle.BackgroundImageNormal = NormalBrush;
	NewStyle.BackgroundImageHovered = HoveredBrush;
	NewStyle.BackgroundImageFocused = FocusedBrush;

#if UE_VERSION_OLDER_THAN(5, 1, 0)
	NewStyle.Font.Size = InputFieldStyleData->FontSize;
#else
	NewStyle.TextStyle.Font.Size = InputFieldStyleData->FontSize;
#endif
	EditableTextBox->WidgetStyle = NewStyle;
}

FInputFieldStyleStateData UCavrnusUIInputField::GetCurrentStateData()
{
	if (InputFieldStyleData == nullptr)
		return FInputFieldStyleStateData();

	return InputFieldStyleData->NormalState;
}
