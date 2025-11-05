// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Menus/MessageMenus/ChatMenu/CavrnusChatWindow.h"

#include "CavrnusFunctionLibrary.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "TimerManager.h"
#include <Misc/EngineVersionComparison.h>

void UCavrnusChatWindow::NativeConstruct()
{
	Super::NativeConstruct();

	SubmitButton->SetIsEnabled(false);
#if UE_VERSION_OLDER_THAN(5, 2, 0)
	DefaultButtonIconColor = SubmitButtonIcon->ColorAndOpacity;
#else
	DefaultButtonIconColor = SubmitButtonIcon->GetColorAndOpacity();
#endif
}

void UCavrnusChatWindow::NativeDestruct()
{
	Super::NativeDestruct();
	
	SubmitButton->OnClicked.RemoveDynamic(this, &UCavrnusChatWindow::SubmitButtonClicked);
	InputTextBox->OnTextChanged.RemoveDynamic(this, &UCavrnusChatWindow::TextInputChanged);
	InputTextBox->OnTextCommitted.RemoveDynamic(this, &UCavrnusChatWindow::TextInputFieldSubmit);
}

void UCavrnusChatWindow::SubmitButtonClicked()
{
	if (!InputTextBox->GetText().IsEmptyOrWhitespace())
		SubmitChat();
}

void UCavrnusChatWindow::TextInputChanged(const FText& InputValue)
{
	const bool ValidInput = !InputValue.IsEmptyOrWhitespace();
	SubmitButton->SetIsEnabled(ValidInput);
	SubmitButtonIcon->SetBrushTintColor(ValidInput ? ButtonEnabledColor : DefaultButtonIconColor);
}

void UCavrnusChatWindow::TextInputFieldSubmit(const FText&, ETextCommit::Type CommitValue)
{
	if (CommitValue == ETextCommit::OnEnter)
		SubmitChat();
}

void UCavrnusChatWindow::SubmitChat()
{
	UCavrnusFunctionLibrary::PostChatMessage(SpaceConnection, InputTextBox->GetText().ToString());

	InputTextBox->SetText(FText::FromString(""));
	InputTextBox->SetKeyboardFocus();
	
	ScrollToEnd();
}

void UCavrnusChatWindow::HandleChatAdded(FChatEntry ChatAdded)
{
	if (ChatAdded.IsTranscription)
		return;
	
	Super::HandleChatAdded(ChatAdded);
}

void UCavrnusChatWindow::HandleChatUpdated(FChatEntry ChatUpdated)
{
	if (ChatUpdated.IsTranscription)
		return;

	Super::HandleChatUpdated(ChatUpdated);
}

void UCavrnusChatWindow::HandleChatRemoved(FString ChatRemoved)
{
	if (AllWidgetsMap.Contains(ChatRemoved))
	{
		Super::HandleChatRemoved(ChatRemoved);
	}
}

void UCavrnusChatWindow::LocalUserArrived()
{
	Super::LocalUserArrived();

	SubmitButton->OnClicked.AddDynamic(this, &UCavrnusChatWindow::SubmitButtonClicked);
	InputTextBox->OnTextChanged.AddDynamic(this, &UCavrnusChatWindow::TextInputChanged);
	InputTextBox->OnTextCommitted.AddDynamic(this, &UCavrnusChatWindow::TextInputFieldSubmit);
}
