// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Widgets/MainEditorTabView/CavrnusMainEditorTabViewWidget.h"

#include "CavrnusConnectorSettings.h"
#include "UI/Helpers/CavrnusWidgetFactory.h"

void UCavrnusMainEditorTabViewWidget::Setup()
{
	const auto ResourcesWidget = FCavrnusWidgetFactory::CreateUserWidget(ResourcesViewBlueprint, GetWorld());
	const auto PreferencesWidget = FCavrnusWidgetFactory::CreateUserWidget(PreferencesViewBlueprint, GetWorld());

	ResourcesWidget->Setup();

	Widgets.Add(ResourcesWidget);
	Widgets.Add(PreferencesWidget);
	
	for (UUserWidget* Widget : Widgets)
		ViewContainer->AddChildToVerticalBox(Widget);
	
	TabHandler = NewObject<UCavrnusUITabHandler>(this);
	TabHandler->Register("Resources", ResourcesButton, ResourcesWidget);

	if (PreferencesButton)
	{
		ClickHandle = PreferencesButton->OnButtonClicked.AddWeakLambda(this, []
		{
			UCavrnusConnectorSettings::Show();
		});
	}
}

void UCavrnusMainEditorTabViewWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (PreferencesButton)
	{
		PreferencesButton->OnButtonClicked.Remove(ClickHandle);
		ClickHandle.Reset();
	}

	if (TabHandler)
		TabHandler->Teardown();

	TabHandler = nullptr;
}
