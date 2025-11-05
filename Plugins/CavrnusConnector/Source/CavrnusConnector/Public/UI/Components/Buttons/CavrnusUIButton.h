// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/CavrnusButtonInterface.h"
#include "UI/Styles/Buttons/CavrnusUIButtonStyle.h"
#include "CavrnusUIButton.generated.h"

class UBorder;
class UButton;
/**
 * Base button class to expose core visual + functionality
 */
UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusUIButton : public UUserWidget, public ICavrnusButtonInterface
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnFocusLost(const FFocusEvent& InFocusEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

public:
	virtual void SetIcon(UTexture2D* IconTexture);
	virtual void SetEnabledState(bool IsEnabled) override;
	virtual void SetVisibleState(bool IsVisible);

	void OverrideButtonStyle(UCavrnusUIButtonStyle* StyleOverride);

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif
	
protected:
	bool IsDisabled = false;
	bool IsPressed = false;
	bool IsHovered = false;
	bool HasKeyboardFocus = false;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus")
	TObjectPtr<UCavrnusUIButtonStyle> ButtonStyleData = nullptr;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Button = nullptr;

	virtual ECavrnusButtonState ResolvePriorityButtonVisualState();

	// Theme handling
	virtual void SynchronizeProperties() override;
	virtual void ApplyButtonStyle();
	virtual void ApplyContentStyle(const FCavrnusUIButtonStyleState&  NewStyle);
	
private:
	UFUNCTION()
	void ButtonClicked();
	UFUNCTION()
	void ButtonPressed();
	UFUNCTION()
	void ButtonReleased();
	UFUNCTION()
	void ButtonHovered();
	UFUNCTION()
	void ButtonUnhovered();
};