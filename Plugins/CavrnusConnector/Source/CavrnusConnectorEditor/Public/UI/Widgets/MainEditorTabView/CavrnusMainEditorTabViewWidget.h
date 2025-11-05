// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusEditorPreferencesViewWidget.h"
#include "CavrnusEditorResourcesViewWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/VerticalBox.h"
#include "UI/Systems/Tabs/CavrnusUITabHandler.h"
#include "CavrnusMainEditorTabViewWidget.generated.h"

UCLASS(Abstract)
class CAVRNUSCONNECTOREDITOR_API UCavrnusMainEditorTabViewWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void Setup();
	
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus")
	TSubclassOf<UCavrnusEditorPreferencesViewWidget> PreferencesViewBlueprint = nullptr;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus")
	TSubclassOf<UCavrnusEditorResourcesViewWidget> ResourcesViewBlueprint = nullptr;

	virtual void NativeDestruct() override;
	
private:
	FDelegateHandle ClickHandle = FDelegateHandle();
	
	UPROPERTY()
	TArray<UUserWidget*> Widgets;
	
	UPROPERTY()
	TObjectPtr<UCavrnusUITabHandler> TabHandler = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> ViewContainer = nullptr;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUIButton> ResourcesButton = nullptr;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UCavrnusUIButton> PreferencesButton = nullptr;
};
