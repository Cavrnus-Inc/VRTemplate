// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/VerticalBox.h"
#include "Modes/CavrnusModeManager.h"
#include "UI/Components/Buttons/Types/CavrnusUIToggleButton.h"
#include "UI/Systems/Panels/CavrnusBasePanelWidget.h"
#include "CavrnusToolbarPanelWidget.generated.h"

USTRUCT()
struct FCavrnusModeButtonBinding
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UCavrnusUIToggleButton> Button = nullptr;

	UPROPERTY()
	TSubclassOf<UCavrnusModeBase> ModeClass = nullptr;
};

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusToolbarPanelWidget : public UCavrnusBasePanelWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UPROPERTY(EditAnywhere, Category="Cavrnus|UI|Toolbar")
	TSubclassOf<UCavrnusUIToggleButton> ToolToggleButtonBlueprint = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> Container;

private:
	UPROPERTY()
	TArray<FCavrnusModeButtonBinding> ButtonBindings;

	UPROPERTY()
	TWeakObjectPtr<UCavrnusModeManager> ModeManager;

	UPROPERTY()
	TObjectPtr<UCavrnusUIToggleButton> ExploreButton;
	UPROPERTY()
	TObjectPtr<UCavrnusUIToggleButton> SceneCaptureButton;
	
	FDelegateHandle ModeDelegate;
};
