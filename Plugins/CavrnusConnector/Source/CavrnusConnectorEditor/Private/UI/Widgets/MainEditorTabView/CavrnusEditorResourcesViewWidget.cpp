// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Widgets/MainEditorTabView/CavrnusEditorResourcesViewWidget.h"
#include "UI/Helpers/CavrnusWidgetFactory.h"

UCavrnusEditorResourcesViewWidget* UCavrnusEditorResourcesViewWidget::Setup()
{
	CreateButton("Visit Documentation", []
	{
		FPlatformProcess::LaunchURL(
			TEXT("https://cavrnus.atlassian.net/wiki/spaces/CSM/pages/872808449/Cavrnus+Spatial+Connector+for+Unreal+Engine"),
			nullptr,
			nullptr);
	});
	CreateButton("Web Console", []
	{
		FPlatformProcess::LaunchURL(
			TEXT("https://console.cavrn.us/"),
			nullptr,
			nullptr);
	});
	CreateButton("Watch Cavrnus On YouTube", []
	{
		FPlatformProcess::LaunchURL(
			TEXT("https://www.youtube.com/@cavrnus"),
			nullptr,
			nullptr);
	});
	CreateButton("Join our Discord Community", []
	{
		FPlatformProcess::LaunchURL(
     			TEXT("https://discord.gg/pjYY8ubf5t"),
     			nullptr,
     			nullptr);
	});

	return this;
}

void UCavrnusEditorResourcesViewWidget::CreateButton(const FString& ButtonText, const TFunction<void()>& OnClicked)
{
	if (TextButtonBlueprint)
	{
		if (auto* Button = FCavrnusWidgetFactory::CreateUserWidget(TextButtonBlueprint, GetWorld()))
		{
			Button->SetButtonText(FText::FromString(ButtonText));
			Button->OnButtonClicked.AddLambda(OnClicked);

			if (ButtonContainer)
				ButtonContainer->AddChild(Button);
		}
	}
}
