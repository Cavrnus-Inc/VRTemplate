// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusWidgetDisplayer.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "CavrnusDesktopCanvasWidgetDisplayer.generated.h"

class UVerticalBox;
class UOverlay;
/**
 * This displayer utilizes a runtime-created canvas and children for each type of displayable widgets (popups, etc)
 */
UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusDesktopCanvasWidgetDisplayer : public UUserWidget, public ICavrnusWidgetDisplayer
{
	GENERATED_BODY()
public:
	virtual void Setup(UCavrnusWidgetBlueprintLookup* InBlueprintLookup = nullptr) override;

	virtual void DisplayRawWidget(const FGuid& Id, UUserWidget* InWidgetToDisplay) override;
	
	virtual void DisplayScrimWidget(
		UCavrnusBaseScrimWidget* InWidgetToDisplay,
		const FCavrnusScrimOptions& Options,
		const TFunction<void(UCavrnusBaseScrimWidget*)>& OnWidgetClosed = nullptr) override;

	virtual void DisplayScreenWidget(
		UCavrnusBaseScreenWidget* InWidgetToDisplay,
		const FCavrnusScreenOptions& Options,
		const TFunction<void(UCavrnusBaseScreenWidget*)>& OnWidgetClosed = nullptr) override;

	virtual void DisplayPanelWidget(
		UCavrnusBasePanelWidget* InWidgetToDisplay,
		const FCavrnusPanelOptions& Options,
		const TFunction<void(UCavrnusBasePanelWidget*)>& OnWidgetClosed = nullptr) override;
	
	virtual void DisplayPopupWidget(
		UCavrnusBasePopupWidget* InUserWidget,
		const FCavrnusPopupOptions& Options,
		const TFunction<void(UCavrnusBasePopupWidget*)>& OnWidgetClosed = nullptr) override;
	
	virtual void DisplayDialogWidget(
		UCavrnusBaseDialogWidget* InUserWidget,
		const FCavrnusDialogOptions& Options,
		const TFunction<void(UCavrnusBaseDialogWidget*)>& OnWidgetClosed = nullptr) override;
	
	virtual void DisplayToastMessageWidget(
		UCavrnusBaseToastMessageWidget* InUserWidget,
		const FCavrnusToastMessageOptions& Options,
		const TFunction<void(UCavrnusBaseToastMessageWidget*)>& OnWidgetClosed = nullptr) override;

	virtual void DisplayLoaderWidget(
		UCavrnusBaseUILoaderWidget* InWidgetToDisplay,
		const FCavrnusUILoaderOptions& Options,
		const TFunction<void(UCavrnusBaseUILoaderWidget*)>& OnWidgetClosed = nullptr) override;

	virtual void RemoveWidget(const FGuid& Id) override;
	virtual void RemoveAll() override;
	
	virtual void NativeDestruct() override;
	virtual void NativeConstruct() override;

private:
	bool bIsInViewport = false;
	void ShouldAddToViewport();

	UPROPERTY()
	TMap<FGuid, TWeakObjectPtr<UUserWidget>> DisplayedWidgets;
	
	UPROPERTY()
	TObjectPtr<UCavrnusWidgetBlueprintLookup> BpLookup = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCanvasPanel> Root = nullptr;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> Popups = nullptr;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> ToastMessages = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> Loaders = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> RawWidgets = nullptr;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> Screens = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> Dialogs = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UPanelWidget> LeftMiddlePanel = nullptr;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UPanelWidget> RightMiddlePanel = nullptr;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UPanelWidget> BottomMiddlePanel = nullptr;
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UPanelWidget> TopMiddlePanel = nullptr;
};