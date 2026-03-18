// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Menus/LoginMenus/CavrnusCombinedLoginWidget.h"

#include "CavrnusConnectorSettings.h"
#include "Components/Overlay.h"
#include "UI/Helpers/CavrnusWidgetFactory.h"
#include "UI/Systems/Tabs/CavrnusUITabHandler.h"

void UCavrnusCombinedLoginWidget::NativeConstruct()
{
	Super::NativeConstruct();

	const auto* Settings = UCavrnusConnectorSettings::Get();
	if (!Settings || !LoginContentArea)
		return;

	// Spawn child widgets from plugin settings
	if (Settings->MemberLoginMenu)
	{
		MemberWidget = FCavrnusWidgetFactory::CreateUserWidget(Settings->MemberLoginMenu, GetWorld());
		if (MemberWidget)
			LoginContentArea->AddChild(MemberWidget);
	}

	if (Settings->GuestJoinMenu)
	{
		GuestWidget = FCavrnusWidgetFactory::CreateUserWidget(Settings->GuestJoinMenu, GetWorld());
		if (GuestWidget)
			LoginContentArea->AddChild(GuestWidget);
	}

	// Set up tab handler for switching between member and guest
	TabHandler = NewObject<UCavrnusUITabHandler>(this);
	TabHandler->SetAllowToggleOff(false);

	if (MemberTabButton && MemberWidget)
		TabHandler->Register(TEXT("Member"), MemberTabButton, MemberWidget);

	if (GuestTabButton && GuestWidget)
		TabHandler->Register(TEXT("Guest"), GuestTabButton, GuestWidget);

	// Activate the preferred tab from plugin settings
	const FString DefaultTab = (Settings->PreferredLoginTab == ECavrnusPreferredLoginTab::Guest) ? TEXT("Guest") : TEXT("Member");
	TabHandler->SetActive(DefaultTab);
}

void UCavrnusCombinedLoginWidget::NativeDestruct()
{
	if (TabHandler)
		TabHandler->Teardown();

	Super::NativeDestruct();
}
