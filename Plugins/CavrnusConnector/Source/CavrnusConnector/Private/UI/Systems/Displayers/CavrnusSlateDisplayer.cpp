// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Displayers/CavrnusSlateDisplayer.h"

#include "Engine/Engine.h"
#include "Widgets/SWindow.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Docking/SDockTab.h" // for ESizingRule::Autosized
#include "UI/Systems/Dialogs/CavrnusBaseDialogWidget.h"
#include "UI/Systems/Popups/CavrnusBasePopupWidget.h"
#include "UI/Systems/Popups/Types/CavrnusPopupSystem.h"

void UCavrnusSlateDisplayer::Setup(UCavrnusWidgetBlueprintLookup* InBlueprintLookup)
{
}

void UCavrnusSlateDisplayer::DisplayPopupWidget(
	UCavrnusBasePopupWidget* InWidgetToDisplay,
	const FCavrnusPopupOptions& Options,
	const TFunction<void(UCavrnusBasePopupWidget*)>& OnWidgetClosed)
{
	ICavrnusWidgetDisplayer::DisplayPopupWidget(InWidgetToDisplay, Options, OnWidgetClosed);
	//
	// if (!InWidgetToDisplay || !GEngine || !GEngine->GameViewport)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("Failed to display popup: Widget, GEngine, or GameViewport is null."));
	// 	return;
	// }
	//
	// TSharedRef<SWidget> PopupContent = InWidgetToDisplay->TakeWidget();
	//
	// TSharedRef<SWindow> PopupWindow = SNew(SWindow)
	// 	.Type(EWindowType::Normal)
	// 	.CreateTitleBar(true)
	// 	.Title(InWidgetToDisplay->GetTitle())
	// 	.HasCloseButton(Options.HasCloseButton)
	// 	.SupportsMaximize(false)
	// 	.SupportsMinimize(false)
	// 	.SizingRule(ESizingRule::Autosized)
	// 	.Content()
	// 	[
	// 		PopupContent
	// 	];
	//
	// PopupWindow->GetOnWindowClosedEvent().AddWeakLambda(this, [OnWidgetClosed, InWidgetToDisplay](const TSharedRef<SWindow>& SharedRef)
	// {
	// 	if (OnWidgetClosed)
	// 		OnWidgetClosed(InWidgetToDisplay);
	// });
	//
	// PopupWindow->SetOnWindowClosed(FOnWindowClosed::CreateLambda([this, InWidgetToDisplay](const TSharedRef<SWindow>&)
	// {
	// 	ActiveSlateWindows.Remove(InWidgetToDisplay->GetId());
	// }));

	// AddModalWindow automatically dims the background and routes all input to this window.
	// if (Options.bIsModal)
	// 	FSlateApplication::Get().AddModalWindow(PopupWindow, GEngine->GameViewport->GetWindow(), false);
	// else
	// 	FSlateApplication::Get().AddWindowAsNativeChild(PopupWindow, GEngine->GameViewport->GetWindow().ToSharedRef(), true);
	//
	// FSlateApplication::Get().SetKeyboardFocus(PopupWindow, EFocusCause::SetDirectly);
	//
	// AllSlateWindows.Add(PopupWindow);
	// ActiveSlateWindows.Add(InWidgetToDisplay->GetId(), PopupWindow);
}

void UCavrnusSlateDisplayer::DisplayDialogWidget(UCavrnusBaseDialogWidget* InWidgetToDisplay,
	const FCavrnusDialogOptions& Options, const TFunction<void(UCavrnusBaseDialogWidget*)>& OnWidgetClosed)
{
	ICavrnusWidgetDisplayer::DisplayDialogWidget(InWidgetToDisplay, Options, OnWidgetClosed);

	// if (!InWidgetToDisplay || !GEngine || !GEngine->GameViewport)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("Failed to display popup: Widget, GEngine, or GameViewport is null."));
	// 	return;
	// }
	//
	// TSharedRef<SWidget> WindowContent = InWidgetToDisplay->TakeWidget();
	// TSharedRef<SWindow> Window = SNew(SWindow)
	// 	.Type(EWindowType::Normal)
	// 	.CreateTitleBar(true)
	// 	.Title(FText::FromString(InWidgetToDisplay->GetTitle()))
	// 	.HasCloseButton(true)
	// 	.SupportsMaximize(false)
	// 	.SupportsMinimize(false)
	// 	.SizingRule(ESizingRule::Autosized)
	// 	.Content()
	// 	[
	// 		WindowContent
	// 	];
	//
	// Window->GetOnWindowClosedEvent().AddWeakLambda(this, [OnWidgetClosed, InWidgetToDisplay](const TSharedRef<SWindow>& SharedRef)
	// {
	// 	if (OnWidgetClosed)
	// 		OnWidgetClosed(InWidgetToDisplay);
	// });
	//
	// Window->SetOnWindowClosed(FOnWindowClosed::CreateLambda([this, InWidgetToDisplay](const TSharedRef<SWindow>&)
	// {
	// 	ActiveSlateWindows.Remove(InWidgetToDisplay->GetId());
	// }));
	//
	// // AddModalWindow automatically dims the background and routes all input to this window.
	// // if (Options.bIsModal)
	// // 	FSlateApplication::Get().AddModalWindow(PopupWindow, GEngine->GameViewport->GetWindow(), false);
	// // else
	// FSlateApplication::Get().AddWindowAsNativeChild(Window, GEngine->GameViewport->GetWindow().ToSharedRef(), true);
	//
	// FSlateApplication::Get().SetKeyboardFocus(Window, EFocusCause::SetDirectly);
	//
	// AllSlateWindows.Add(Window);
	// ActiveSlateWindows.Add(InWidgetToDisplay->GetId(), Window);
}

void UCavrnusSlateDisplayer::RemoveWidget(const FGuid& Id)
{
	// if (TSharedPtr<SWindow>* FoundWindow = ActiveSlateWindows.Find(Id))
	// {
	// 	if (GEngine && GEngine->GameViewport && FoundWindow->IsValid())
	// 	{
	// 		FoundWindow->ToSharedRef()->RequestDestroyWindow();
	// 		ActiveSlateWindows.Remove(Id);
	// 	}
	// }
}

void UCavrnusSlateDisplayer::RemoveAll()
{
	// for (const TSharedPtr Window : AllSlateWindows)
	// 	Window->RequestDestroyWindow();
	//
	// AllSlateWindows.Empty();
	// ActiveSlateWindows.Empty();
}