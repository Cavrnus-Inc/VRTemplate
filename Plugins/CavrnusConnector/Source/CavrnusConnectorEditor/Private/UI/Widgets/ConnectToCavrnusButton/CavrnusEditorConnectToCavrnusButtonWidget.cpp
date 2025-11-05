// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Widgets/ConnectToCavrnusButton/CavrnusEditorConnectToCavrnusButtonWidget.h"
#include "CavrnusSubsystem.h"

void UCavrnusEditorConnectToCavrnusButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	auto ConnManager = UCavrnusSubsystem::Get()->Services->Get<UCavrnusEditorAuthenticationManager>();
	if (ConnManager == nullptr)
		return;

	if (Button == nullptr)
		return;
	
	ConnStateChangedHandle = ConnManager->BindConnectedState([this](const ECavrnusEditorConnectedStateEnum& State)
	{
		HandleConnectionState(State);
	});
	
	AuthStartedHandle = ConnManager->OnAuthStarted.AddWeakLambda(this,[this]
	{
		Button->SetEnabledState(false);
		SetPrimaryText("Waiting for connection...", true);
		SetSecondaryText("Sign into Cavrnus from your web browser", true);
	});

	ButtonClickedHandle = Button->OnButtonClicked.AddWeakLambda(this, [ConnManager]
	{
		ConnManager->DoConnect();
	});
}

void UCavrnusEditorConnectToCavrnusButtonWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (const auto Sc = UCavrnusSubsystem::Get()->Services->Get<UCavrnusEditorAuthenticationManager>())
	{
		Sc->OnAuthStarted.Remove(AuthStartedHandle);
		Sc->OnConnStateChanged.Remove(ConnStateChangedHandle);
		
		AuthStartedHandle.Reset();
		ConnStateChangedHandle.Reset();
	}

	if (Button)
	{
		Button->OnButtonClicked.Remove(ButtonClickedHandle);
		ButtonClickedHandle.Reset();
	}
}

void UCavrnusEditorConnectToCavrnusButtonWidget::HandleConnectionState(const ECavrnusEditorConnectedStateEnum& CurrentState)
{
	switch (CurrentState)
	{
	case Connected:
		SetPrimaryText("", false);
		SetSecondaryText("", false);
		SetRichText("", false);
		Button->SetEnabledState(false);
		Button->SetVisibleState(false);
		Button->SetButtonText(FText::FromString("Connected"));
		ButtonContainer->SetVisibility(ESlateVisibility::Collapsed);
		Throbber->SetVisibility(ESlateVisibility::Collapsed);
		ConnectionSuccessBox->SetVisibility(ESlateVisibility::Visible);
		break;
	case Connecting:
		SetPrimaryText("Waiting for connection...", true);
		Button->SetEnabledState(false);
		Button->SetButtonText(FText::FromString("Connecting..."));
		Throbber->SetVisibility(ESlateVisibility::Visible);
		ConnectionSuccessBox->SetVisibility(ESlateVisibility::Collapsed);
		break;
	case Disconnected:
		SetPrimaryText("", false);
		SetSecondaryText("", false);
		SetRichText("", false);
		Button->SetEnabledState(true);
		Button->SetVisibleState(true);
		Button->SetButtonText(FText::FromString("Sign In or Sign Up"));
		ButtonContainer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		Throbber->SetVisibility(ESlateVisibility::Collapsed);
		ConnectionSuccessBox->SetVisibility(ESlateVisibility::Collapsed);
		break;
	default:
		break;
	}
}

void UCavrnusEditorConnectToCavrnusButtonWidget::SetPrimaryText(const FString& InText, const bool bVis)
{
	if (PrimaryConnectionStatusTextBlock)
	{
		PrimaryConnectionStatusTextBlock->SetText(FText::FromString(InText));
		PrimaryConnectionStatusTextBlock->SetVisibility(bVis ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UCavrnusEditorConnectToCavrnusButtonWidget::SetSecondaryText(const FString& InText, const bool bVis)
{
	if (SecondaryConnectionStatusTextBlock)
	{
		SecondaryConnectionStatusTextBlock->SetText(FText::FromString(InText));
		SecondaryConnectionStatusTextBlock->SetVisibility(bVis ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UCavrnusEditorConnectToCavrnusButtonWidget::SetRichText(const FString& InText, const bool bVis)
{
	if (RichTextBlock)
	{
		RichTextBlock->SetText(FText::FromString(InText));
		RichTextBlock->SetVisibility(bVis ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}
