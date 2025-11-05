// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/VerticalBox.h"
#include "UI/Components/Buttons/Types/CavrnusUITextButton.h"
#include "CavrnusEditorResourcesViewWidget.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class CAVRNUSCONNECTOREDITOR_API UCavrnusEditorResourcesViewWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UCavrnusEditorResourcesViewWidget* Setup();
	
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus")
	TSubclassOf<UCavrnusUITextButton> TextButtonBlueprint;

private:
	UPROPERTY()
	TArray<TObjectPtr<UCavrnusUITextButton>> Buttons;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> ButtonContainer = nullptr;

	void CreateButton(const FString& ButtonText, const TFunction<void()>& OnClicked);
};
