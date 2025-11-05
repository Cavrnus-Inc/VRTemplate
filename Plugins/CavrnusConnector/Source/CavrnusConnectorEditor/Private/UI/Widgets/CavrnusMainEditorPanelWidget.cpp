// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Widgets/CavrnusMainEditorPanelWidget.h"

#include "CavrnusSubsystem.h"
#include "RestAPI/CavrnusRestHelpers.h"
#include "UI/Helpers/CavrnusWidgetFactory.h"

bool UCavrnusMainEditorPanelWidget::ProjectSetupWidgetVis = true;
TMulticastDelegate<void(UCavrnusMainEditorPanelWidget* Widget)> UCavrnusMainEditorPanelWidget::OnConstructed;
void UCavrnusMainEditorPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (OnConstructed.IsBound())
		OnConstructed.Broadcast(this);
	
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	
	// I think this should be hidden if already connected...
	if (ConnectToCavrnusButtonBlueprint)
	{
		ConnectToCavrnusButtonWidget = FCavrnusWidgetFactory::CreateUserWidget<UCavrnusEditorConnectToCavrnusButtonWidget>(ConnectToCavrnusButtonBlueprint, World);
		AddToContainer(ConnectToCavrnusButtonWidget);
	}
	
	if (AuthenticationSelectionBlueprint)
	{
		AuthenticationSelectionWidget = FCavrnusWidgetFactory::CreateUserWidget<UCavrnusAuthenticationSelectionWidget>(AuthenticationSelectionBlueprint, World);
		AuthenticationSelectionWidget->Setup();
		AddToContainer(AuthenticationSelectionWidget);
	}

	if (ProjectSetupBlueprint && ProjectSetupWidgetVis)
	{
		ProjectSetupWizardWidget = FCavrnusWidgetFactory::CreateUserWidget<UCavrnusEditorProjectSetupWizard>(ProjectSetupBlueprint, World);
		AddToContainer(ProjectSetupWizardWidget);
	}

	// Show when connected
	if (ModifierBlueprint)
	{
		// TODO:: We are hiding this for now! No code components are changing...
		
		// ModifierVM = NewObject<UCavrnusComponentModifierViewModel>(this);
		// ModifierVM->Setup();
		// 		
		// ModifierWidget = FCavrnusWidgetFactory::CreateUserWidget<UCavrnusComponentModifierMainWidget>(ModifierBlueprint, World);
		// ModifierWidget->Setup(ModifierVM);
		// 		
		// AddToContainer(ModifierWidget);
	}
			
	if (TabViewBlueprint)
	{
		TabViewWidget = FCavrnusWidgetFactory::CreateUserWidget<UCavrnusMainEditorTabViewWidget>(TabViewBlueprint, World);
		TabViewWidget->Setup();

		if (HeaderContainer)
			HeaderContainer->AddChildToVerticalBox(TabViewWidget);
	}
	
	ConnStateHandle = UCavrnusSubsystem::Get()->Services->Get<UCavrnusEditorAuthenticationManager>()->BindConnectedState([this](const ECavrnusEditorConnectedStateEnum& State)
	{
		switch (State)
		{
		case Connected:
			AuthenticationSelectionWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			break;
		case Connecting:
			break;
		case Disconnected:
			AuthenticationSelectionWidget->SetVisibility(ESlateVisibility::Collapsed);
			break;
		}
	});
}

void UCavrnusMainEditorPanelWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (const auto Cm = UCavrnusSubsystem::Get()->Services->Get<UCavrnusEditorAuthenticationManager>())
	{
		Cm->OnConnStateChanged.Remove(ConnStateHandle);
		ConnStateHandle.Reset();
	}
	
	ModifierWidget = nullptr;
	ConnectToCavrnusButtonWidget = nullptr;
}

void UCavrnusMainEditorPanelWidget::AddToContainer(UUserWidget* Widget)
{
	if (ContainerVerticalBox)
		ContainerVerticalBox->AddChildToVerticalBox(Widget);
}

void UCavrnusMainEditorPanelWidget::InjectWidget(UUserWidget* WidgetToInject)
{
	if (InjectedContainerVerticalBox)
	{
 		InjectedContainerVerticalBox->AddChildToVerticalBox(WidgetToInject);
		InjectedWidgets.Add(WidgetToInject); // take ownership
	}
}

void UCavrnusMainEditorPanelWidget::SetProjectSetupWidgetVis(const bool bVis)
{
	ProjectSetupWidgetVis = bVis;
	if (ProjectSetupWizardWidget)
		ProjectSetupWizardWidget->SetVisibility(bVis ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}

void UCavrnusMainEditorPanelWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	SetProjectSetupWidgetVis(false);
}

void UCavrnusMainEditorPanelWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	FCavrnusRestHelpers::PostHasSeenSetupScreenMetric();
}
