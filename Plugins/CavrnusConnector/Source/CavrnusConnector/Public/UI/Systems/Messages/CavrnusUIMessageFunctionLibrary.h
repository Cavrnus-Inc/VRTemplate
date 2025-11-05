// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusScopedMessages.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToastMessages/CavrnusToastMessageUISystem.h"
#include "ToastMessages/Info/CavrnusInfoToastMessageWidget.h"
#include "ToastMessages/Progress/CavrnusProgressToastMessageWidget.h"
#include "UI/CavrnusUI.h"
#include "CavrnusUIMessageFunctionLibrary.generated.h"

class UCavrnusUISubsystem;
/**
 * Exposes concrete message types to blueprints
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusUIMessageFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="Cavrnus|UI|Messages")
	static UCavrnusInfoToastMessageWidget* CreateInfoToastMessage(
		const bool AutoClose = false,
		const float CloseDuration = 3.0f)
	{
		return AutoClose ?
			UCavrnusUI::Get()->Messages()->Toast()->CreateAutoClose<UCavrnusInfoToastMessageWidget>() : 
			UCavrnusUI::Get()->Messages()->Toast()->Create<UCavrnusInfoToastMessageWidget>();
	}

	UFUNCTION(BlueprintCallable, Category="Cavrnus|UI|Messages")
	static UCavrnusProgressToastMessageWidget* CreateProgressToastMessage(
		const bool AutoClose = false,
		const float CloseDuration = 3.0f)
	{
		return AutoClose ?
			UCavrnusUI::Get()->Messages()->Toast()->CreateAutoClose<UCavrnusProgressToastMessageWidget>() : 
			UCavrnusUI::Get()->Messages()->Toast()->Create<UCavrnusProgressToastMessageWidget>();
	}
};
