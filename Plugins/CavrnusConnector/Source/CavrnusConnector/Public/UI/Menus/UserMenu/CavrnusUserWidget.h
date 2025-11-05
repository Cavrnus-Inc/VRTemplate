// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include <Blueprint/UserWidget.h>
#include "Types/CavrnusBinding.h"
#include "Types/CavrnusCallbackTypes.h"

#include "CavrnusUserWidget.generated.h"

class UTexture2D;

typedef TFunction<void(FCavrnusUser&)> FMaximizeUserSelected;

/**
 * @brief Base class for widget representing a Cavrnus User in the UI.
 *
 * This class provides the necessary functionality to display user-related information and
 * handle user-specific actions within the Cavrnus platform's UI.
 */
UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusUserWidget : public UUserWidget
{
	GENERATED_BODY()
	/**
	 * @brief Destructor override to handle cleanup when the widget is destroyed.
	 */
	virtual void NativeDestruct() override;

public:
	/**
	 * @brief Initializes the user connection with the given space connection and user data.
	 * @param InSpaceConnection The space connection associated with the user.
	 * @param InUser The user data to initialize the connection.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cavrnus|UI")
	void InitializeUserConnection(const FCavrnusUser& InUser);

	UFUNCTION(BlueprintImplementableEvent)
	void InitializeComplete();

	/** @brief Image widget to display the RTC stream of the user. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|UI", meta = (BindWidget))
	class UImage* RtcStreamImage = nullptr;

	/** @brief User data associated with the widget. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|UI")
	FCavrnusUser User = FCavrnusUser();

	FMaximizeUserSelected MaximizeUserSelected;

private:

	UFUNCTION(BlueprintCallable, Category = "Cavrnus|UI")
	void TriggerMaximizeUserSelected();
	
	/**
	 * @brief Binds the user video to the widget.
	 */
	UFUNCTION()
	void BindUserVideo();
	/**
	 * @brief Unbinds the user video from the widget.
	 */
	UFUNCTION()
	void UnbindUserVideo();

	/** @brief Binding for the user's video frame updates. */
	UPROPERTY()
	UCavrnusBinding* UserVideoFrameBinding = nullptr;

	/** @brief Current size of the RTC stream image. */
	FVector2D CurrentStreamImageSize = FVector2D(0,0);
	
	/**
	 * @brief Checks if the texture size has changed.
	 * @param NewSize The new size of the texture.
	 * @return True if the size has changed, false otherwise.
	 */
	bool TextureSizeChanged(FVector2D NewSize) const;
};
