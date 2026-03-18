// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/VerticalBox.h"
#include "Modes/CavrnusModeManager.h"
#include "UI/Components/Buttons/Types/CavrnusUIToggleButton.h"
#include "UI/Systems/Panels/CavrnusBasePanelWidget.h"
#include "CavrnusToolbarPanelWidget.generated.h"

// Forward declarations
class UCavrnusToolbarPanelWidget;
class UCavrnusUIToggleButton;

// Delegate for modules to extend the toolbar
DECLARE_MULTICAST_DELEGATE_OneParam(FOnToolbarConstructed, UCavrnusToolbarPanelWidget*);

USTRUCT()
struct FCavrnusModeButtonBinding
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UCavrnusUIToggleButton> Button = nullptr;

	UPROPERTY()
	TSubclassOf<UCavrnusModeBase> ModeClass = nullptr;
};

// Button descriptor for registration
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusToolbarButtonDescriptor
{
	GENERATED_BODY()

	// Icon name to lookup in IconsDataAsset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|UI|Toolbar")
	FString IconName;

	// Insert position (INDEX_NONE to append at end)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|UI|Toolbar")
	int32 InsertIndex = INDEX_NONE;

	// Function to create and configure the button
	// Returns the created button, or nullptr if creation should be skipped
	TFunction<UCavrnusUIToggleButton*(UCavrnusToolbarPanelWidget* ToolbarWidget)> ButtonFactory;
};

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusToolbarPanelWidget : public UCavrnusBasePanelWidget
{
	GENERATED_BODY()
public:
	// Extension API for modules to add buttons dynamically
	UFUNCTION(BlueprintCallable, Category="Cavrnus|UI|Toolbar")
	void AddToolbarButton(UCavrnusUIToggleButton* Button, int32 InsertIndex = -1); // No support for INDEX_NONE in UFUNCTIONs

	// Get the button blueprint template for creating new buttons
	TSubclassOf<UCavrnusUIToggleButton> GetToolToggleButtonBlueprint() const { return ToolToggleButtonBlueprint; }

	// Static delegate that fires when any toolbar widget is constructed
	// Modules can bind to this to add their buttons
	static FOnToolbarConstructed OnToolbarConstructed;
	
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