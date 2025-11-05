// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusBaseEditorUtilityWidget.h"
#include "AuthenticationSelectionWidget/CavrnusAuthenticationSelectionWidget.h"
#include "ComponentModifierWidget/CavrnusComponentModifierMainWidget.h"
#include "ComponentModifierWidget/CavrnusComponentModifierViewModel.h"
#include "Components/VerticalBox.h"
#include "ConnectToCavrnusButton/CavrnusEditorConnectToCavrnusButtonWidget.h"
#include "MainEditorTabView/CavrnusMainEditorTabViewWidget.h"
#include "ProjectSetupWizard/CavrnusEditorProjectSetupWizard.h"
#include "CavrnusMainEditorPanelWidget.generated.h"

/**
 * Main panel which houses all of the sub panels
 */
UCLASS(Abstract)
class CAVRNUSCONNECTOREDITOR_API UCavrnusMainEditorPanelWidget : public UCavrnusBaseEditorUtilityWidget
{
	GENERATED_BODY()
public:
	static TMulticastDelegate<void(UCavrnusMainEditorPanelWidget* Widget)> OnConstructed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus|MainEditorPanel")
	TSubclassOf<UCavrnusComponentModifierMainWidget> ModifierBlueprint;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus|MainEditorPanel")
	TSubclassOf<UCavrnusEditorProjectSetupWizard> ProjectSetupBlueprint;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus|MainEditorPanel")
	TSubclassOf<UCavrnusEditorConnectToCavrnusButtonWidget> ConnectToCavrnusButtonBlueprint;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus|MainEditorPanel")
	TSubclassOf<UCavrnusAuthenticationSelectionWidget> AuthenticationSelectionBlueprint;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus|MainEditorPanel")
	TSubclassOf<UCavrnusMainEditorTabViewWidget> TabViewBlueprint;

	void InjectWidget(UUserWidget* WidgetToInject);
	void SetProjectSetupWidgetVis(const bool bVis);
	
protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	static bool ProjectSetupWidgetVis;
	
	UPROPERTY()
	TArray<UUserWidget*> InjectedWidgets;
	
	FDelegateHandle ConnStateHandle = FDelegateHandle();
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> HeaderContainer = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> ContainerVerticalBox = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> InjectedContainerVerticalBox = nullptr;

	UPROPERTY()
	TObjectPtr<UCavrnusEditorProjectSetupWizard> ProjectSetupWizardWidget = nullptr;
	
	UPROPERTY()
	TObjectPtr<UCavrnusComponentModifierMainWidget> ModifierWidget = nullptr;
	UPROPERTY()
	TObjectPtr<UCavrnusComponentModifierViewModel> ModifierVM = nullptr;

	UPROPERTY()
	TObjectPtr<UCavrnusEditorConnectToCavrnusButtonWidget> ConnectToCavrnusButtonWidget = nullptr;

	UPROPERTY()
	TObjectPtr<UCavrnusAuthenticationSelectionWidget> AuthenticationSelectionWidget = nullptr;

	UPROPERTY()
	TObjectPtr<UCavrnusMainEditorTabViewWidget> TabViewWidget = nullptr;
	
	void AddToContainer(UUserWidget* Widget);
};
