// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Messages/ToastMessages/ServerStatus/CavrnusServerStatusToastWidget.h"

#include "UI/Components/Borders/CavrnusUIBorder.h"
#include "UI/Components/Image/CavrnusUIImage.h"
#include "UI/Components/Text/CavrnusUITextBlock.h"

void UCavrnusServerStatusToastWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	SetStatusInternal(CurrentStatus);
}

void UCavrnusServerStatusToastWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	
	if (ThemeAsset)
		SetStatusInternal(CurrentStatus);
}

UCavrnusServerStatusToastWidget* UCavrnusServerStatusToastWidget::SetStatus(const FCavrnusConnectionStatus& InStatus)
{
	SetIcon(ThemeAsset->GetTheme(InStatus.Status).Icon);

	if (PrimaryBorder)
		PrimaryBorder->SetBrushColor(ThemeAsset->GetTheme(InStatus.Status).PrimaryColor);

	SetStatusInternal(InStatus.Status, InStatus.ErrorReason);
	
	return this;
}

UCavrnusServerStatusToastWidget* UCavrnusServerStatusToastWidget::SetPrimaryText(const FString& InPrimaryText)
{
	if (PrimaryText)
		PrimaryText->SetText(FText::FromString(InPrimaryText));
	
	return this;
}

UCavrnusServerStatusToastWidget* UCavrnusServerStatusToastWidget::SetSecondaryText(const FString& InPrimaryText)
{
	if (SecondaryText)
		SecondaryText->SetText(FText::FromString(InPrimaryText));
	
	return this;
}

UCavrnusServerStatusToastWidget* UCavrnusServerStatusToastWidget::SetIcon(UTexture2D* InIcon)
{
	if (Icon)
		Icon->SetBrushFromTexture(InIcon);
	
	return this;
}

void UCavrnusServerStatusToastWidget::SetStatusInternal(const ECavrnusConnectionStatusEnum& Status, const FString& Message)
{
	CurrentStatus = Status;

	switch (Status)
	{
	case ECavrnusConnectionStatusEnum::ReadyToConnect:
		SetPrimaryText("Ready");
		SetSecondaryText("Waiting to connect");
		break;
	case ECavrnusConnectionStatusEnum::Connecting:
		SetPrimaryText("Connecting");
		SetSecondaryText("Establishing connection...");
		break;
	case ECavrnusConnectionStatusEnum::ProcessingInitialState:
		SetPrimaryText("Loading");
		SetSecondaryText("Syncing initial state...");
		break;
	case ECavrnusConnectionStatusEnum::Live:
		SetPrimaryText("Connected");
		SetSecondaryText("Live in the space");
		break;
	case ECavrnusConnectionStatusEnum::Reconnecting:
		SetPrimaryText("Reconnecting");
		SetSecondaryText("Restoring connection...");
		break;
	case ECavrnusConnectionStatusEnum::Closed:
		SetPrimaryText("Disconnected");
		SetSecondaryText("Connection closed");
		break;
	case ECavrnusConnectionStatusEnum::Error:
		SetPrimaryText("Connection Error");
		SetSecondaryText(Message);
		break;
	}
}
