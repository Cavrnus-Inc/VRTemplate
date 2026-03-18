// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusScopedMessages.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToastMessages/CavrnusToastMessageUISystem.h"
#include "ToastMessages/Info/CavrnusInfoToastMessageWidget.h"
#include "ToastMessages/Progress/CavrnusProgressToastMessageWidget.h"
#include "UI/CavrnusUI.h"
#include "UI/CavrnusUISystems.h"
#include "CavrnusUIMessageFunctionLibrary.generated.h"

class UCavrnusUISystems;
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

	/**
	 * @brief Create an info toast with all fields set in one call.
	 * @param PrimaryText The main heading text
	 * @param SecondaryText The detail/body text
	 * @param Type The toast type (Info, Success, Warning, Error)
	 * @param AutoClose Whether to auto-dismiss the toast
	 * @param CloseDuration Auto-dismiss duration in seconds
	 * @return The created info toast widget (for further customization if needed)
	 */
	UFUNCTION(BlueprintCallable, Category="Cavrnus|UI|Messages",
		meta = (ToolTip = "Create an info toast message with all fields set"))
	static UCavrnusInfoToastMessageWidget* CreateCompleteInfoToastMessage(
		const FString& PrimaryText,
		const FString& SecondaryText,
		ECavrnusInfoToastMessageEnum Type = ECavrnusInfoToastMessageEnum::Info,
		const bool AutoClose = true,
		const float CloseDuration = 3.0f);

	/**
	 * @brief Create a progress toast with all fields set in one call (progress bar starts hidden).
	 * @param PrimaryText The main heading text
	 * @param SecondaryText The detail/body text
	 * @param Type The toast type (Info, Success, Warning, Error)
	 * @param AutoClose Whether to auto-dismiss the toast
	 * @param CloseDuration Auto-dismiss duration in seconds
	 * @return The created progress toast widget (call SetProgress to update the bar)
	 */
	UFUNCTION(BlueprintCallable, Category="Cavrnus|UI|Messages",
		meta = (ToolTip = "Create a progress toast message with all fields set"))
	static UCavrnusProgressToastMessageWidget* CreateCompleteProgressToastMessage(
		const FString& PrimaryText,
		const FString& SecondaryText,
		ECavrnusInfoToastMessageEnum Type = ECavrnusInfoToastMessageEnum::Info,
		const bool AutoClose = false,
		const float CloseDuration = 3.0f);
};
