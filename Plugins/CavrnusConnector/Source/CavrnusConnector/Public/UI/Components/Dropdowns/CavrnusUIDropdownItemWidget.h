// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusRuntimeDropdownEntry.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "UI/Components/Buttons/CavrnusUIButton.h"
#include "UI/Components/Text/CavrnusUITextBlock.h"
#include "CavrnusUIDropdownItemWidget.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusUIDropdownItemWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	FCavrnusRuntimeDropdownEntry Data = FCavrnusRuntimeDropdownEntry();
	
	void Setup(const FCavrnusRuntimeDropdownEntry& InData);
	
private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCavrnusUIButton> Button;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCavrnusUITextBlock> DisplayNameTextBlock;	
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Icon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Checkmark;
};
