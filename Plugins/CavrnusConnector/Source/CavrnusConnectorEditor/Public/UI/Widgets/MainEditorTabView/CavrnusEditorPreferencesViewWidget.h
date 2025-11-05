// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Components/Buttons/Types/CavrnusUITextButton.h"
#include "CavrnusEditorPreferencesViewWidget.generated.h"

UCLASS(Abstract)
class CAVRNUSCONNECTOREDITOR_API UCavrnusEditorPreferencesViewWidget : public UUserWidget
{
	GENERATED_BODY()
	
	FDelegateHandle ClickHandle = FDelegateHandle();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCavrnusUIButton> Button;
};
